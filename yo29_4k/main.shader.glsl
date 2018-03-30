#version 130
uniform float F[1];
float t = F[0];
uniform sampler2D S;
vec3 E = vec3(0.,.01,1.);
vec4 noise24(vec2 v) { return texture2D(S, (v + .5)/textureSize(S,0)); }

vec4 cseed, macroseed;
float world(vec3 p) {
	float d = p.y - 400.;
	//if (d > 10.)
	//	return d + 10.;
	p /= 100.;
	d = p.y - 2.*noise24(p.xz/10.).x;
	vec2 C = floor(p.xz), TC = C;
	float min1 = 10., min2 = 10.;
	for (float x = -1.; x <= 1.; x+=1.)
	for (float y = -1.; y <= 1.; y+=1.) {
		vec2 cn = C + vec2(x, y), c = cn + .5;
		vec2 macrocell = floor(cn / 10.);
		macroseed = noise24(macrocell);
		cseed = noise24(cn);
		c += (cseed.yz - .5) * macroseed.x;
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

	cseed = noise24(TC);
	macroseed = noise24(floor(TC / 10.));
	float height = .1 + 3. * macroseed.y;// * macroseed.y;
	d = min(d, max(p.y - cseed.x * height, (.3 - vd) * .5));
	return 100. * max(d, length(p)-200.);
}

vec3 normal(vec3 p) {
	return normalize(vec3(
		world(p+E.yxx), world(p+E.xyx), world(p+E.xxy)) - world(p));
}

float march(vec3 o, vec3 d, float l, float maxl) {
	float mind=10., minl = l;
	for (int i = 0; i < 99; ++i) {
		float dd = world(o + d * l);
		l += dd * .7;
		if (dd < .001 * l || l > maxl) return l;

		//if (l > maxl) break;

		if (dd < mind) {
			mind = dd; minl = l;
		}

	}
	return minl;
}

vec3 O, D, N, P;
vec4 m_diffk = vec4(1.,1.,1.,.5);
float m_shine = 10.;

float dirlight(vec3 ld) {
	return mix(
		max(0., dot(N, ld)),
		max(0., pow(dot(N, normalize(ld - D)), m_shine) * (m_shine + 8.) / 24.),
		m_diffk.w);
}
vec3 sundir = normalize(vec3(1.,1.+sin(t*.1),1.));

const float R0 = 6360e3;
const float Ra = 6380e3;
const int steps = 64;
const int stepss = 32;
const float g = .76;
const float g2 = g * g;
const float Hr = 8e3;
const float Hm = 1.2e3;
const float I = 10.;

vec3 C = vec3(0., -R0, 0.);
vec3 bR = vec3(58e-7, 135e-7, 331e-7);
vec3 bM = vec3(21e-6);

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
		.225 * noise31(v*2. + t *.4) +
		.125 * noise31(v*3.99) +
		.0625 * noise31(v*8.9);
}

float cloud(vec3 p) {
	float cld = fnoise(p*2e-4);
	cld = smoothstep(.4+.04, .6+.04, cld);
	cld *= cld * 40.;
	return cld;
}

vec2 densitiesRM(vec3 pos) {
	float h = max(0., length(pos - C) - R0);
	vec2 retRM = vec2(exp(-h/Hr), exp(-h/Hm));

	const float low = 5e3;
	if (low < h && h < 10e3) {
		retRM.y += cloud(pos+vec3(23175.7, 0.,-t*3e3))
			* max(0., sin(3.1415*(h-low)/low));
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
			vec3 A = exp(-(bR * depthRMsum.x + bM * depthRMsum.y));
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

	return Lo * exp(-(bR * totalDepthRM.x + bM * totalDepthRM.y))
		+ I * (LiR * bR * phaseRM.x + LiM * bM * phaseRM.y);
}

void main() {
	const vec2 res = vec2(640.,360.);
	vec2 uv = gl_FragCoord.xy/res * 2. - 1.; uv.x *= res.x / res.y;

	vec3 at = vec3(0.);

	O = vec3(-1.000,4.000,8.000) * 100.; //10. * vec3(sin(t*.1), 0., cos(t*.1)) + vec3(0., 3. + 2. * sin(t*.2), 0.);
	D = -normalize(vec3(0.700,-0.170,0.670));//normalize(at - O);
	vec3 x = normalize(cross(normalize(vec3(0.,1.,0.)), D));
	D = mat3(x, normalize(cross(D, x)), D) * normalize(vec3(uv, -1.));
	vec3 color = vec3(0.);

	const float md = 5e4;
	float l = march(O, D, 0., md);
	if (l < md) {
		P = O + D * l;
		N = normal(P);

		float flr = mod(P.y, 3.)/3.; //max(0., sin(P.y*40.));
		m_shine = 10. + flr * 90.;
		//color = (vec3(1.) + .04 *cseed.xyz) * (.5 + .5 * flr);
		//color = macroseed.rgb;
		color = cseed.rgb;
		color *= dirlight(sundir);
	} else {
		l = escape(O, D, Ra);
	}

	color = scatter(O, D, l, color);

	gl_FragColor = vec4(pow(color, vec3(1./2.2)),0.);
}
