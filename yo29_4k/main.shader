//#version 130
//uniform int F[1];
float t = $(float time);
uniform sampler2D S;
vec3 E = vec3(0.,.01,1.);
vec4 noise24(vec2 v) { return texture2D(S, (v + .5)/textureSize(S,0)); }

const float R0 = 6360e3;
const float Ra = 6380e3;
const int steps = 128;
const int stepss = 16;
const float g = .76;
const float g2 = g * g;
const float Hr = 8e3;
const float Hm = 1.2e3;
float I = 10.;

const vec3 C = vec3(0., -R0, 0.);
const vec3 bR = vec3(58e-7, 135e-7, 331e-7);
const vec3 bMs = vec3(2e-5);
const vec3 bMe = bMs * 1.1;

vec3 sundir = vec3(1.,.001,1.);

/*
float noise31(vec3 v) {
	vec3 p = floor(v);
  vec3 f = fract(v);
	f = f*f*(3.-2.*f);

	vec2 uv = (p.xy+vec2(37.,197.)*p.z) + f.xy;
	vec2 rg = textureLod(S, (uv+.5)/textureSize(S,0), 0.).yx;
	return mix(rg.x, rg.y, f.z);
}*/

float noise31(vec3 v) {
	return mix(noise24(v.xz).x, noise24(v.yx).y, .5);// fract(v.z));
}

float fnoise(vec3 v) {
	return
		.55 * noise31(v)
		+ .225 * noise31(v*1.8)
		+ .125 * noise31(v*4.3)
		//+ .0625 * noise31(v*8.9)
		;
}

float cloud(vec3 p) {
	float cld = fnoise(p*6e-4);
	cld = smoothstep(.4, .6, cld-.05);
	return cld;
}

bool clouds = true;
vec2 densitiesRM(vec3 p) {
	float h = max(0., length(p - C) - R0);
	vec2 retRM = vec2(exp(-h/Hr), exp(-h/Hm));

	if (clouds) {
		/*
		float r = length(p.xz);
		if (r < 100. && p.y < 500.) {
			retRM.y += 6e3 * smoothstep(.35, .55, fnoise(p*5e-2-vec3(0.,t*2.,0.))) * step(r, 8. + .3 * (p.y - 200.));
		}
		*/

		float low = 5e3, hi = 10e3, border = 1e3;
		if (low < h && h < hi) {
			retRM.y +=
				50. *
				step(length(p), 50000.) *
				smoothstep(low, low + border, h) *
				//smoothstep(hi, hi - border, h) *
				cloud(p+vec3(0., 0.,t*30.));
		}
	}

	return retRM;
}

float escape(vec3 p, vec3 d, float R) {
	vec3 v = p - C;
	float b = dot(v, d);
	float c = dot(v, v) - R*R;
	float det2 = b * b - c;
	if (det2 < 0.) return -1.;
	float det = sqrt(det2);
	float t1 = -b - det, t2 = -b + det;
	return (t1 >= 0.) ? t1 : t2;
}

vec3 Lin(vec3 o, vec3 d, float L) {
	float dls = L / float(stepss);
	vec2 depthRMs = vec2(0.);
	for (int j = 0; j < stepss; ++j) {
		vec3 ps = o + d * float(j) * dls;
		depthRMs += densitiesRM(ps) * dls;
	}
	return exp(-(bR * depthRMs.x + bMe * depthRMs.y));
}

vec3 scatter(vec3 o, vec3 d, float L, vec3 Lo) {
	vec2 totalDepthRM = vec2(0., 0.);
	vec3 LiR = vec3(0.), LiM = vec3(0.);

	float l = 0.;
	for (int i = 0; i < steps; ++i) {
		float fi = float(i + 1) / float(steps);
		fi *= fi;
		fi *= fi;
		float lp = L * fi;
		float dl = lp - l;
		vec3 p = o + d * lp;
		l = lp;
		vec2 dRM = densitiesRM(p) * dl;
		totalDepthRM += dRM;

		float Ls = escape(p, sundir, Ra);
#if 1
		float dls = Ls / float(stepss);
#else
		float ls = 0.;
#endif
		vec2 depthRMs = vec2(0.);
		for (int j = 0; j < stepss; ++j) {
#if 0
			float fj = float(j + 1) / float(stepss);
			fj *= fj;
			float lps = Ls * fj;
			float dls = lps - ls;
			ls = lps;
			vec3 ps = p + sundir * lps;
#else
			vec3 ps = p + sundir * float(j) * dls;
#endif
			depthRMs += densitiesRM(ps) * dls;
		}

		vec2 depthRMsum = depthRMs + totalDepthRM;
		vec3 A = exp(-(bR * depthRMsum.x + bMe * depthRMsum.y));
		LiR += A * dRM.x;
		LiM += A * dRM.y;
	}

	float mu = dot(d, sundir);
	float opmu2 = 1. + mu*mu;
	vec2 phaseRM = vec2(
		.0596831 * opmu2,
		.1193662 * (1. - g2) * opmu2 / ((2. + g2) * pow(1. + g2 - 2.*g*mu, 1.5)));

	return Lo * exp(-(bR * totalDepthRM.x + bMe * totalDepthRM.y))
		+ I * (LiR * bR * phaseRM.x + LiM * bMs * phaseRM.y);
}

float saturate(float f) { return clamp(f, 0., 1.); }
float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }
float box(vec3 p, vec3 s) { return vmax(abs(p) - s); }
vec3 rep(vec3 p, vec3 s) { return mod(p, s) - s*.5; }
vec2 rep(vec2 p, vec2 s) { return mod(p, s) - s*.5; }

float mindex = 0.;

float building(vec3 p, vec2 cell) {
	//return 0.;
	vec4 rnd = noise24(cell);
	vec4 rnd2 = noise24(cell/8.);

	vec3 s = vec3(
		10. + rnd.x * 30.,
		3. * (2. + floor(pow(rnd2.z,3.) * 100.)),
		10. + rnd.y * 30.);
	return box(p, s);
}

float world(vec3 p) {
	float bound = length(p) - 1e4;

	if (p.y > 250.) return p.y - 200.;

	float d = p.y;//max(p.y, length(p) - 1e3);
	mindex = 0.;

	vec2 cell = floor(p.xz / 100.);
	p.xz = rep(p.xz, vec2(100.));
	//float bx = box(p, vec3(40., 200., 40.));
	//float bx = box(p, vec3(50., 200., 50.));
	float bx = max(abs(p.x)-50., abs(p.z)-50.);
	if (bx < -5.) {
		d = min(min(d, -bx+5.), building(p, cell));
	}
	else
		d = min(d, bx+10.);

/*
	float floors = 4.;
	float bld = box(p, vec3(6., floors * 3., 20.));
	float glass = bld + .2;
	bld = max(bld, -box(rep(p+vec3(1.,0.,0.), vec3(2.)), vec3(.5)));
	bld = min(bld,
		max(bld-1.,
			box(rep(p+vec3(0.,1.,0.),vec3(12., 3., 10.)), vec3(.5, .5, 1.5))));

	if (bld < d) {
		d = bld;
		mindex = 1.;
	}

	if (glass < d) {
		d = glass;
		mindex = 2.;
	}
	*/

	return max(d, bound);
}

float march(vec3 o, vec3 d, float l, float maxl) {
	for (int i = 0; i < 200; ++i) {
		float dd = world(o + d * l);
		l += dd;
		if (dd < .002 * l || l > maxl) return l;
	}
	return maxl;
}

void main() {
	const vec2 res = vec2(640., 360.)*2.;
	vec2 uv = gl_FragCoord.xy/res * 2. - 1.; uv.x *= res.x / res.y;

	if (gl_FragCoord.y < 10.) {
		gl_FragColor = vec4(step(gl_FragCoord.x / res.x, t / 208.));
		return;
	}

	//gl_FragColor = noise24(gl_FragCoord.xy);return;
	//t += noise24(gl_FragCoord.xy + t*100.*vec2(17.,39.)).x;

	vec3 O, D, N, P;
	vec3 at = vec3(0.);
	O = vec3(mod(t*2., 10000.) - 5000., 1000. + 500. * sin(t/60.), mod(t*10., 10000.) - 5000.);

	if (t < 64.) {
		I = 10. * t / 64.;
		at = vec3(10000., 15000., -10000.);
	} else if (t < 128.) {
		O.y = 1000.;
		float k = (t - 64.) / 64.;
		O = vec3(k * 1000., 1500., 3000.);
		at = O+vec3(-300., 0., -300.);
		at.y = 10.;
		sundir.y += .01 * k * k * k;
	} else if (t < 144.) {
		sundir.y = .01;
	} else {
		float k = (t - 144.) / 64.;
		O = vec3(mod(k*2., 10000.) - 500., 1000. - 200. * k, mod(k*10., 10000.) - 5000.);
		at.y = 4000. * k;
		sundir.y = .01 + .5* k;// * k * k * k;
		I = 10. + 200. * max(0., (t - 196.) / 16.);
	}
	sundir = normalize(sundir);

	D = normalize(O - at);
	O = $(vec3 cam_pos) * 10.; D = -normalize($(vec3 cam_dir));
	vec3 x = normalize(cross(E.xzx, D));
	D = mat3(x, normalize(cross(D, x)), D) * normalize(vec3(uv, -1.));
	vec3 color = vec3(0.);

	const float max_distance = 1e4;
	vec3 color_coeff = vec3(1.);
	//for (int i = 0; i < 1; ++i)
	{
		float l = march(O, D, .1, max_distance);
		vec3 localcolor = vec3(0.);
		vec3 m_emissive = vec3(0.);
		float m_kd = .5;
		float m_shine = 10.;
		if (l < max_distance) {
			P = O + D * l;
			N = normalize(vec3(
				world(P+E.yxx), world(P+E.xyx), world(P+E.xxy)) - world(P));

			vec3 albedo = vec3(.2, .6, .1);
			m_shine = 0.;

			if (mindex == 1.) {
				albedo = vec3(.2);
			}
			if (mindex == 2.) {
				albedo = vec3(1.);
				m_shine = 10000.;
			}

			localcolor = albedo * Lin(P, sundir, escape(P, sundir, Ra)) * I
				* mix(
					max(0., dot(N, sundir)) / 3.,
					max(0., pow(dot(N, normalize(sundir - D)), m_shine) * (m_shine + 8.) / 24.),
					m_kd);
			vec3 opposite = vec3(-sundir.x, sundir.y, -sundir.z);
			clouds = false;
			localcolor += albedo * scatter(P, opposite, escape(P, opposite, Ra), vec3(0.)) * m_kd * max(0., dot(N, opposite)) / 3.;
			clouds = true;
			localcolor += m_emissive;
			color += color_coeff * scatter(O, D, l, localcolor);
			D = reflect(D, N);
			O = P;
			//color_coeff *= albedo;
		} else {
			l = escape(O, D, Ra);
			color += color_coeff * scatter(O, D, l, localcolor);
	//		break;
		}
	}

	gl_FragColor = vec4(pow(color, vec3(1./2.2)),.5);
}
