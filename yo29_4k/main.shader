//#version 130
//uniform int F[1];
float t = $(float time);
uniform sampler2D S;
vec3 E = vec3(0.,.01,1.);
vec4 noise24(vec2 v) { return texture2D(S, (v + .5)/textureSize(S,0)); }

const float R0 = 6360e3;
const float Ra = 6380e3;
const float low = 2e3, hi = 4e3, border = 1e3;
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
		.75 * noise31(v)
		+ .125 * noise31(v*4.3)
		//+ .0625 * noise31(v*9.9)
		//+ .0625 * noise31(v*17.8)
		;
}

float cloud(vec3 p) {
	float cld = fnoise(p*1e-3);
	cld = smoothstep(.49, .5, cld);
	return cld;
}

bool clouds = true;
vec2 densitiesRM(vec3 p) {
	float h = max(0., length(p - C) - R0);
	vec2 retRM = vec2(exp(-h/Hr), exp(-h/Hm) * 9.);

	if (clouds) {
		/*
		float r = length(p.xz);
		if (r < 100. && p.y < 500.) {
			retRM.y += 6e3 * smoothstep(.35, .55, fnoise(p*5e-2-vec3(0.,t*2.,0.))) * step(r, 8. + .3 * (p.y - 200.));
		}
		*/

		if (low < h && h < hi) {
			retRM.y +=
				50. *
				step(length(p), 50000.) *
				smoothstep(low, low + border, h) *
				smoothstep(hi, hi - border, h) *
				cloud(p+vec3(0., 0.,t*80.));
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

vec2 scatterDirectImpl(vec3 o, vec3 d, float L, float steps) {
	vec2 depthRMs = vec2(0.);
	L /= steps; d *= L;
	//o += d * noise24(o.xz*1e3 + vec2(t*1e4)).x;
	for (float i = 0.; i < steps; ++i)
		//depthRMs += densitiesRM(o + d * (i + noise24(vec2(i,i)).x));
		depthRMs += densitiesRM(o + d * i);
	return depthRMs * L;
}

vec3 Lin(vec3 o, vec3 d, float L) {
	vec2 depthRMs = scatterDirectImpl(o, d, L, 32.);
	return exp(-(bR * depthRMs.x + bMe * depthRMs.y));
}

void scatterImpl(vec3 o, vec3 d, float L, float steps, inout vec2 totalDepthRM, inout vec3 LiR, inout vec3 LiM) {
	L /= steps; d *= L;
	for (float i = 0.; i < steps; ++i) {
		vec3 p = o + d * i;
		p += d * noise24(p.xz).w;
		//vec3 p = o + d * i;(i + noise24(vec2(i,i)).x)
		vec2 dRM = densitiesRM(p) * L;
		totalDepthRM += dRM;

		/*
		//vec2 depthRMsum = scatterDirectImpl(p, sundir, escape(p, sundir, Ra), 32.) + totalDepthRM;
		vec2 depthRMsum = totalDepthRM;
		float l = escape(p, sundir, R0 + low);
		if (l > 0.)
			depthRMsum += scatterDirectImpl(p, sundir, l, 4.);
		else l = 0.;
		float l2 = escape(p, sundir, R0 + hi);
		if (l2 > 0.)
			depthRMsum += scatterDirectImpl(p + sundir * l, sundir, l2, 16.);
		else l2 = 0.;
		l = escape(p, sundir, Ra);
		if (l > 0.)
			depthRMsum += scatterDirectImpl(p + sundir * l2, sundir, l, 8.);
		*/

		vec2 depthRMsum = totalDepthRM;
		float l = escape(p, sundir, R0 + hi);
		if (l > 0.)
			depthRMsum += scatterDirectImpl(p, sundir, l, 16.);
		else l = 0.;
		depthRMsum += scatterDirectImpl(p + sundir * l, sundir, escape(p, sundir, Ra), 4.);

		//vec2 depthRMsum = totalDepthRM + scatterDirectImpl(p, sundir, escape(p, sundir, Ra), 32.);

		/*
		float lesc = escape(p, sundir, Ra);
		vec2 depthRMsum = totalDepthRM + scatterDirectImpl(p, sundir, lesc, min(128., lesc / 100.));
		*/


		vec3 A = exp(-(bR * depthRMsum.x + bMe * depthRMsum.y));
		LiR += A * dRM.x;
		LiM += A * dRM.y;
	}
}

vec3 scatter(vec3 o, vec3 d, float L, vec3 Lo) {
	vec2 totalDepthRM = vec2(0.);
	vec3 LiR = vec3(0.), LiM = vec3(0.);

	/*
	float det_dist = 1e4;
	float left = L - det_dist;
	if (left > 0.) {
		scatterImpl(o, d, det_dist, 64., totalDepthRM, LiR, LiM);
		scatterImpl(o + d * det_dist, d, left, 32., totalDepthRM, LiR, LiM);
	} else
		scatterImpl(o, d, L, 32., totalDepthRM, LiR, LiM);
		*/

	float l = escape(o, d, R0 + low);//, l2 = escape(o, d, R0 + hi);
	if (L < l)
		scatterImpl(o, d, L, 16., totalDepthRM, LiR, LiM);
	else {
		float l2 = escape(o, d, R0 + hi);
		scatterImpl(o, d, l, 32., totalDepthRM, LiR, LiM);
		scatterImpl(o+d*l, d, l2-l, 32., totalDepthRM, LiR, LiM);
		scatterImpl(o+d*l2, d, L-l2, 8., totalDepthRM, LiR, LiM);
	}

	float mu = dot(d, sundir);
	return Lo * exp(-(bR * totalDepthRM.x + bMe * totalDepthRM.y))
		+ I * (1. + mu * mu) * (
			LiR * bR * .0596831 +
			LiM * bMs * .1193662 * (1. - g2) / ((2. + g2) * pow(1. + g2 - 2. * g * mu, 1.5)));
}


float saturate(float f) { return clamp(f, 0., 1.); }
float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }
float box(vec3 p, vec3 s) { return vmax(abs(p) - s); }
vec3 rep(vec3 p, vec3 s) { return mod(p, s) - s*.5; }
vec2 rep(vec2 p, vec2 s) { return mod(p, s) - s*.5; }

float mindex = 0.;

float dbg = 0.;
float building(vec3 p, vec2 cell) {
	vec4 rnd = noise24(cell);
	vec4 rnd2 = noise24(cell/8.);

	float floor_height = 3.;

	vec3 s = vec3(
		5. + rnd.x * 7.,
		3. * (2. + floor(rnd.z * 10. + 30. * smoothstep(.8,.99, rnd.z) * smoothstep(.6, .9, rnd2.z))),
		5. + rnd.y * 7.);
	return min(
		box(p, s-.2),
		max(p.y - s.y, box(vec3(p.x, mod(p.y/*+floor_height*.25*/,floor_height) - floor_height*.5, p.z), vec3(s.x, .3, s.z)))
	);
}

float world(vec3 p) {
	float bound = length(p) - 1e4;
	float highway = abs(dot(normalize(vec2(-1.,2.)),p.xz)) - 12.;

	float d = p.y;//max(p.y, length(p) - 1e3);
	mindex = 1.;

	vec2 cell = floor(p.xz / 30.);
	p.xz = rep(p.xz, vec2(30.));
	float bx = max(abs(p.x)-15., abs(p.z)-15.);
	float guard = 2.;
	dbg = bx;
	if (bx < -guard)
		d = min(min(d, -bx+guard), building(p, cell));
		//box(p,vec3(12.*sin(cell.x)))));
	else
		d = min(d, -bx+guard);

	d = max(d, -highway);
	return max(min(d,p.y), bound);
}

float march(vec3 o, vec3 d, float l, float maxl) {
	for (int i = 0; i < 800; ++i) {
		float dd = world(o + d * l);
		l += dd;
		if (dd < .0003 * l  || l > maxl) return l;
	}
	return maxl;
}

void main() {
	const vec2 res = vec2(640., 360.);//*2.;
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
		//sundir.y = .01 + .5* k;// * k * k * k;
		sundir.y = .01 + 2.* k;// * k * k * k;
		//I = 10. + 200. * max(0., (t - 196.) / 16.);
	}
	sundir = normalize(sundir);

	D = normalize(O - at);
	O = $(vec3 cam_pos) * 10.; D = -normalize($(vec3 cam_dir));
	vec3 x = normalize(cross(E.xzx, D));
	D = mat3(x, normalize(cross(D, x)), D) * normalize(vec3(uv, -2.));
	vec3 color = vec3(0.);

	const float max_distance = 1e4;
	vec3 color_coeff = vec3(1.);
	//for (int i = 0; i < 1; ++i)
	{
		float start_l = .1;// (O.y - 200.) / D.y;
		//float start_l = O.y < 200. ? .1 : (200. - O.y) / D.y;
		float l = march(O, D, start_l, max_distance);
		vec3 localcolor = vec3(0.);
		vec3 m_emissive = vec3(0.);
		float m_kd = .3;
		float m_shine = 10.;
		if (l < max_distance) {
			P = O + D * l;
			N = normalize(vec3(
				world(P+E.yxx), world(P+E.xyx), world(P+E.xxy)) - world(P));

			vec3 albedo = vec3(.2, .6, .1);
			m_shine = 10.;

			if (mindex == 1.) {
				m_kd = .3;
				m_shine = 80.;
				albedo = vec3(.2);
			}
			if (mindex == 2.) {
				albedo = vec3(1.);
				m_shine = 10000.;
			}

			//albedo = fract(P);
			//color = vec3(-dbg/30.);
			//albedo = vec3(-dbg/30.);

			localcolor = albedo * Lin(P, sundir, escape(P, sundir, Ra)) * I
				* mix(
						max(0., dot(N, sundir)) / 3.,
						max(0., pow(dot(N, normalize(sundir - D)), m_shine) * (m_shine + 8.) / 24.),
						m_kd
					);
			vec3 opposite = vec3(-sundir.x, sundir.y, -sundir.z);
			clouds = false;
			localcolor += albedo * scatter(P, opposite, escape(P, opposite, Ra), vec3(0.)) * m_kd * max(0., dot(N, opposite)) / 3.;
			clouds = true;
			localcolor += m_emissive;
			color += color_coeff * scatter(O, D, l, localcolor);
		} else {
			l = escape(O, D, Ra);
			color += color_coeff * scatter(O, D, l, localcolor);
	//		break;
		}
	}

	//color = vec3(-N.y);
	//gl_FragColor = color.x < 0.0001 ? vec4(1.,0.,0.,1.) : vec4(pow(color, vec3(1./2.2)),.5);
	gl_FragColor = vec4(pow(color, vec3(1./2.2)),.5);
}
