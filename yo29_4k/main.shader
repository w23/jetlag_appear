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

vec4 cseed, macroseed;
vec3 lc;
float walls;
float park;
float world(vec3 p) {
	float d = p.y - 400.;
	//if (d > 10.)
	//	return d + 10.;
	float r = length(p.xz);
	p /= 100.;
	vec2 C = floor(p.xz), TC = C;
	float min1 = 10., min2 = 10.;
	for (float x = -1.; x <= 1.; x+=1.)
	for (float y = -1.; y <= 1.; y+=1.) {
		vec2 cn = C + vec2(x, y), c = cn + .5;
		vec2 macrocell = floor(cn / 10.);
		macroseed = noise24(macrocell);
		float center_dist2 = dot(cn,cn);
		cseed = noise24(cn);
		c += (cseed.yz - .5) * smoothstep(100., 200., center_dist2) * macroseed.x;
		//float dist = length(c - p.xz);
		float dist = abs(c.x - p.x) + abs(c.y - p.z);
		if (dist < min1) {
			min2 = min1;
			min1 = dist;
			TC = cn;
		} else if (dist < min2)
			min2 = dist;
	}

	float vd = min2 - min1;

	float center_dist2 = dot(TC, TC);
	cseed = noise24(TC);
	macroseed = noise24((TC / 10.));
	lc = p - vec3(TC.x, 0., TC.y);
	park = step(.8, macroseed.w);
	float maxh = smoothstep(10000., 1000., r) * (1. - park);
	float height = maxh * (1. + 4. * smoothstep(.8, 1., macroseed.x));
	walls = (.3 - vd) * .5;
	d = min(p.y, max(p.y - cseed.x * height, walls));
	return 100. * max(d, length(p)-100.);
}

vec3 normal(vec3 p) {
	return normalize(vec3(
		world(p+E.yxx), world(p+E.xyx), world(p+E.xxy)) - world(p));
}

float march(vec3 o, vec3 d, float l, float maxl) {
	float mind=10., minl = l;
	for (int i = 0; i < 199; ++i) {
		float dd = world(o + d * l);
		l += dd * .37;
		if (dd < .001 * l || l > maxl) return l;

		//if (l > maxl) break;

		if (dd < mind) {
			mind = dd; minl = l;
		}

	}
	return minl;
}

vec3 O, D, N, P;
float m_kd = .5;
float m_shine = 10.;

float dirlight(vec3 ld) {
	return mix(
		max(0., dot(N, ld)) / 3.,
		max(0., pow(dot(N, normalize(ld - D)), m_shine) * (m_shine + 8.) / 24.),
		m_kd);
}
vec3 sundir = vec3(1.,.001,1.);

// by iq
float noise31(in vec3 v) {
	vec3 p = floor(v);
  vec3 f = fract(v);
	//f = f*f*(3.-2.*f);

	vec2 uv = (p.xy+vec2(37.,17.)*p.z) + f.xy;
	vec2 rg = textureLod(S, (uv+.5)/textureSize(S,0), 0.).yx;
	return mix(rg.x, rg.y, f.z);
}

float fnoise(in vec3 v) {
	return
		.55 * noise31(v) +
		.225 * noise31(v*2.) +
		.125 * noise31(v*3.99) +
		.0625 * noise31(v*8.9);
}

float cloud(vec3 p) {
	float cld = fnoise(p*4e-4);
	cld = smoothstep(.4+.04, .6+.04, cld);
	cld *= cld * 40.;
	return cld;
}

vec2 densitiesRM(vec3 pos) {
	float h = max(0., length(pos - C) - R0);
	vec2 retRM = vec2(exp(-h/Hr), exp(-h/Hm));

	const float low = 5e3;
	if (low < h && h < 10e3) {
		retRM.y += step( length(pos), 50000.) * cloud(pos+vec3(23175.7, 0.,t*30.)) * max(0., sin(3.1415*(h-low)/low));
	}

	return retRM;
}

float escape(in vec3 p, in vec3 d, in float R) {
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

// this can be explained: http://www.scratchapixel.com/lessons/3d-advanced-lessons/simulating-the-colors-of-the-sky/atmospheric-scattering/
vec3 scatter(vec3 o, vec3 d, float L, vec3 Lo) {
	vec2 totalDepthRM = vec2(0., 0.);
	vec3 LiR = vec3(0.), LiM = vec3(0.);

	float dl = L / float(steps);
	for (int i = 0; i < steps; ++i) {
		vec3 p = o + d * float(i) * dl;
		vec2 dRM = densitiesRM(p) * dl;
		totalDepthRM += dRM;

		float Ls = escape(p, sundir, Ra);
		if (Ls > 0.) {
			float dls = Ls / float(stepss);
			vec2 depthRMs = vec2(0.);
			for (int j = 0; j < stepss; ++j) {
				vec3 ps = p + sundir * float(j) * dls;
				depthRMs += densitiesRM(ps) * dls;
			}

			vec2 depthRMsum = depthRMs + totalDepthRM;
			vec3 A = exp(-(bR * depthRMsum.x + bMe * depthRMsum.y));
			LiR += A * dRM.x;
			LiM += A * dRM.y;
		} else {
			return vec3(0.);
		}
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

void main() {
	const vec2 res = vec2(640.,360.);
	vec2 uv = gl_FragCoord.xy/res * 2. - 1.; uv.x *= res.x / res.y;

	if (gl_FragCoord.y < 10.) {
		gl_FragColor = vec4(step(gl_FragCoord.x / res.x, t / 208.));
		return;
	}

	//gl_FragColor = noise24(gl_FragCoord.xy);return;

	t += noise24(gl_FragCoord.xy + t*100.*vec2(17.,39.)).x;

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
	//O = $(vec3 cam_pos) * 100.; D = -normalize($(vec3 cam_dir));
	vec3 x = normalize(cross(E.xzx, D));
	D = mat3(x, normalize(cross(D, x)), D) * normalize(vec3(uv, -1.));
	vec3 color = vec3(0.);

	const float md = 5e4;
	float l = march(O, D, 0., md);
	vec3 emissive = vec3(0.);
	if (l < md) {
		P = O + D * l;
		N = normal(P);

		vec3 albedo = vec3(.3 + .2 * cseed.w) + .03 * cseed.xyz;
		float occlusion = 1. - .3 * saturate(.5 * ((10. - P.y) / 10. + (1. - walls) / 1.));
		//occlusion = 1.;
		m_shine = 200.;
		if (park > 0.) {
			float path = step(.1, walls);
			albedo = .05 * mix(vec3(.4,.9,.3) - vec3(.2 * noise24(P.xz).x), .8*vec3(.6,.8,.2), path);
			m_kd = .5 - .5 * (1. - path);
			m_shine = 10. * (1. - path);
			occlusion = 1.;
		} else if (P.y < 1.) {
			albedo = vec3(.01 + .04 * step(walls, .07)) + vec3(.7) * step(.145, walls);//mix(vec3(.05), vec3(.01), step(3., walls));
			m_shine = 1.;
		} else {
			//float a = atan(lc.x, lc.z);
			//albedo *= .8 + .2 * (step(2., mod(P.y, 4.)) * step(2., mod(lc.x*100., 4.)));
		}

		albedo *= occlusion;

		//emissive += (1. - occlusion) * vec3(.4, .3, .1);

		//vec4 rnd = noise24(vec2(floor(local_coord.y/2.), floor(atan(mp.x, mp.z) * 1000.)));
		//	color = vec3(1., 0., 1.);
		//color = macroseed.rgb;
		//color = cseed.rgb;
		color += albedo * Lin(P, sundir, escape(P, sundir, Ra)) * I * dirlight(sundir);
		vec3 opposite = vec3(-sundir.x, sundir.y, -sundir.z);
		color += albedo * scatter(P, opposite, escape(P, opposite, Ra), vec3(0.)) * dirlight(opposite);
		color += albedo * vec3(.07) * dirlight(normalize(vec3(1.)));
		color += emissive;
	} else {
		l = escape(O, D, Ra);
	}

	color = scatter(O, D, l, color);

	gl_FragColor = vec4(pow(color, vec3(1./2.2)),.5);
}
