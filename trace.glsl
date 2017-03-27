uniform float uf_time;
uniform vec2 uv2_resolution;
uniform sampler2D us2_rand;
uniform vec2 uv2_rand_resolution;
varying vec2 vv2_pos;

float t = uf_time;

vec4 rnd(vec2 n) { return texture2D(us2_rand, n/uv2_rand_resolution); }
float hash(float n) { return texture2D(us2_rand, vec2(n*17.,n*53.)/uv2_rand_resolution).x; }
float hash(vec2 n) { return texture2D(us2_rand, n*17./uv2_rand_resolution).y; }
float noise(vec2 v) { return texture2D(us2_rand, (v+.5)/uv2_rand_resolution).z; }
float noise(float v) { return noise(vec2(v)); }

float fbm(vec2 v) {
	float r = 0.;
	float k = .5;
	for (int i = 0; i < 5; ++i) {
		r += noise(v) * k;
		k *= .5;
		v *= 2.;
	}
	return r;
}

float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }

float box(vec3 p, vec3 s) {
	return vmax(abs(p) - s);
}

float tor(vec3 p, vec2 s) {
	return length(vec2(length(p.xz) - s.x, p.y)) - s.y;
}

mat3 rotY(float a) {
	float c = cos(a), s = sin(a);
	return mat3(c, 0., s, 0., 1., 0., -s, 0., c);
}

mat3 rotX(float a) {
	float c = cos(a), s = sin(a);
	return mat3(1., 0., 0., 0., c, -s, 0., s, c);
}

vec3 rep(vec3 p, vec3 x) {
	return mod(p, x) - x*.5;
}

float bounds(vec3 p) {
	return -box(p+vec3(0.,-20.,0.), vec3(20.));
}

float object(vec3 p) {
	p -= vec3(0., 3., 0.);

	float cd = dot(p, vec3(1.,0.,0.)*rotY(t));
	const float cr = .4, cs = .02;
	float cut = abs(mod(cd,cr)-cr*.5)-cs;

	//p.x += sin(p.y*2. + t);

	float t = t * .3;
#if 1
	p.xy = abs(p.yx); p *= rotX(t);
	p.xz = abs(p.zx); p *= rotX(t*.7);
#if 0
	/* INTERESTING */ p.zy = abs(p.yx); p *= rotY(t*1.1);
#endif
	p.zy = abs(p.yz); p *= rotY(t*1.1);
#endif

	float d = min(min(min(
			box(p, vec3(.7,2.4,.2)),
			box(p, vec3(1.3,.4,.3))),
			box(rotX(.6)*p, vec3(.3,.4,2.3))),
			tor(p, vec2(1., .1))
			);

	return max(d, -cut);
}

float world(vec3 p) {
	return min(bounds(p), object(p));
}

struct material_t {
	vec3 emissive;
	vec3 color;
	float roughness;
	float specular;
	float fresnel;
	float metalness;
};

material_t material(vec3 p) {
	material_t m;
#if 0
	m.emissive = vec3(0.);
	m.color = vec3(1.);
	m.roughness = 1.;
	m.specular = 0.;
	m.fresnel = 0.;
	m.metalness = 0.;
#else
	if (object(p) < bounds(p)) {
		m.emissive = vec3(2.0) * smoothstep(.99,.999,sin(t*4.*.4+(rotX(t*.4)*p).y*2.));
		m.color = vec3(.56,.57,.58);
		m.roughness = .01 + .5 * fbm(p.xy*10.);
		m.specular = .8;
		m.fresnel = .4;
		m.metalness = .5 + fbm(p.zx*21.)*.5;
	} else {
		float type = smoothstep(.4,.6,fbm(p.xz*4.));
		m.emissive = vec3(0.);
		m.color = vec3(.07);
		m.roughness = mix(1., .01, type);
		m.specular = mix(.1, .9, type);
		m.fresnel = .02;
		m.metalness = mix(.01, .9, type);
	}
#endif
	return m;
}

vec3 normal(vec3 p) {
	vec2 e = vec2(0., .001);
	float d = world(p);
	return normalize(vec3(world(p+e.yxx) - d, world(p+e.xyx) - d, world(p+e.xxy) - d));
}

const float E = .005;


vec3 trace(vec3 O, vec3 D, float L, float Lmax) {
	float Lp = L;
	for (int i = 0; i < 64; ++i) {
		vec3 p = O + D * L;
		float d = world(p);
		if (d < 0.) return vec3(Lp, L, 0.);
		Lp = L;
		L += max(d, E * L * .5);
		if (L >= Lmax) return vec3(Lmax, Lmax, 0.);
	}
	return vec3(-1., L, 0.);
}

vec3 refine(vec3 a, vec3 b) {
	for (int i = 0; i < 8; ++i) {
		vec3 m = (a + b) * .5;
		if (world(m) < 0.) b = m; else a = m;
	}
	return (a + b) * .5;
}


const float PI = 3.1415926;

vec3 brdf(vec3 N, vec3 L, vec3 V, material_t m) {
	vec3 H = normalize(L + V);
	float NL = max(0., dot(N, L));
	float NH = max(0., dot(N, H));
	float LH = max(0., dot(L, H));
	float r2 = m.roughness * m.roughness;

	vec3 F0 = mix(vec3(m.fresnel), m.color, m.metalness);
	vec3 F = F0 + (vec3(1.) - F0) * pow(1. - LH, 5.);
	float D = r2 / (PI * pow(NH * NH * (r2 - 1.) + 1., 2.));
	float G = 1. / (LH * LH);

	vec3 Fd = F0 + (vec3(1.) - F0) * pow(1. - NL, 5.);
	return PI * mix((1. - Fd)/PI, F * D * G / 4., m.specular) * m.color * NL;
}

vec3 enlight(vec3 pos, vec3 normal, vec3 view, vec3 lightpos, vec3 lightcolor) {
	material_t m = material(pos);
	vec3 lightdir = lightpos - pos;
	vec3 nld = normalize(lightdir);
	float ldist = length(lightdir);
	float d = 1.;//trace(pos + normal*E*10., nld, E, ldist).z;
	const int shadow_samples = 40;
	float L = E;
	for (int i = 0; i < shadow_samples; ++i) {
		vec3 p = pos + normal*.1 + nld * L;
		float w = world(p);
		d = min(d, w);
		if (w < .0) break;
		L += max(w, E * L);
		if (L >= ldist) break;
	}
	return pow(smoothstep(0., .06, d), 2.) * brdf(normal,nld,view,m) * lightcolor / dot(lightdir, lightdir);
}

vec3 directlight(vec3 p, vec3 v) {
	vec3 n = normal(p);
	material_t m = material(p);
	vec3 color = m.emissive;
	color += enlight(p, n, v, vec3(5., 5. , 5.), 200.*vec3(.4, .3, .9));
	color += enlight(p, n, v, vec3(8., 8., -8.), 20.*vec3(.2, .8, .3));
	color += enlight(p, n, v, vec3(-7., 7., 7.), 20.*vec3(.7, .3, .9));
	return color;
}

vec3 eye(float t) {
	t *= .7;
	const vec3 C = vec3(0., 3., 0.), S = 2.*vec3(6., 2., 6.);
	float T = floor(t); t -= T; t *= t * (3. - 2. * t);
	t = 1. - pow(1. - t, 2.);
#if 0
	vec3 A = (rnd(vec2(T)).wyz - .5) * S,
			 B = rnd(vec2(T+1.)).wyz;
#else
	vec3 A = (rnd(vec2(T)).wyz - .5) * S,
			 B = (rnd(vec2(T+1.)).wyz - .5) * S;
#endif
	vec3 p = mix(A, B, t);
	return C + normalize(p) * max(length(p), 2.4);
}

void main() {
	//t = uf_time * .1;
	vec2 uv = vv2_pos * vec2(1., uv2_resolution.y / uv2_resolution.x);

	//gl_FragColor = texture2D(us2_rand, vv2_pos*.5+.5); return;
	//gl_FragColor = texture2D(us2_rand, gl_FragCoord.xy/uv2_rand_resolution); return;
	//gl_FragColor = vec4(noise(gl_FragCoord.xy)); return;
	//gl_FragColor = vec4(noise(uv*10.)); return;
	//gl_FragColor = rnd(gl_FragCoord.xy); return;

	vec3 color = vec3(0.);

	//vec3 at = vec3(2.2,3.,0.);
	vec3 at = vec3(0., 3., 0.);
	//vec3 eye = vec3(3.7, 4.5, 4.7) + .1 * vec3(noise(t), noise(t*.4), noise(t*.7));
	vec3 eye = eye(t);
	vec3 up = vec3(0., 1., 0.);
	vec3 dir = normalize(at - eye);
	vec3 side = cross(up, dir); up = cross(dir, side);
	mat3 lat = mat3(side, up, -dir);
	
	vec3 o = vec3(uv, 0.);
	vec3 d = normalize(vec3(uv, -1.));

	o = lat * o + eye;
	d = lat * d;

#if 1
	float Lmax = 30.;
	vec3 ll = trace(o, d, 0., Lmax);
	if (ll.x > 0. && ll.x < Lmax) {
		vec3 p = refine(o + d * ll.x, o + d * ll.y);
		vec3 n = normal(p);
		material_t m = material(p);
		color = directlight(p, -d);

		int steps = 7;
		//float ksteps = pow(1. / float(steps), 2.);
		float ksteps = 1. / float(steps);
		for (int i = 0; i < steps; ++i) {
			float seed = float(i) + t;
			vec3 sd = normalize(
				mix(reflect(d,n),
					n + normalize(2. * (rnd(1000.*(p.xz+seed+p.y)).zyx - .5)),
				1.-m.specular));
			vec3 os = p + n * E * 2.;
			float Lm = 4.;
			vec3 l = trace(os, sd, 0., Lm);
			if (l.x > 0. && l.x < Lm) {
				//vec3 ps = refine(os + sd * l.x, os + sd * l.y);
				vec3 ps = os + sd * l.x;
				material_t sm = material(ps);
				color += 0.
#if 0
					+ sm.emissive
#else
					+ /* brdf(n, sd, -d, m) */  directlight(ps, -sd)
#endif
					* ksteps  / max(1.,l.x); // float(steps);
				;
			}
		}
	}
#elif 0
	float Lmax = 30.;
	vec3 O = o, D = d, N;
	material_t M;
	const int steps = 9;
	float kstep = 1.;// / float(steps);
	for (int s = 0; s < steps; ++s) {
		float L = 0.;
		float Lp = L;
		vec3 p;
		for (int i = 0; i < 64; ++i) {
			p = O + D * L;
			float d = world(p);
			if (d < 0.) break;
			Lp = L;
			L += max(d, E * L * .5);
			if (L >= Lmax) break;
		}

		if (L >= Lmax) {
			if (s == 0) break;
			continue;
		}

		vec3 n = normal(p);
		material_t m = material(p);
		vec3 c = vec3(.0);
		c += enlight(p, n, -D, vec3(5., 5. , 5.), 200.*vec3(1., .3, .2));
		c += enlight(p, n, -D, vec3(8., 8., -8.), 200.*vec3(.2, .8, .3));
		c += enlight(p, n, -D, vec3(-7., 7., 7.), 200.*vec3(.7, .3, .9));
		color += c * kstep;

		if (s == 0) {
			O = p + n * E * 2.;
			N = n;
			M = m;
			kstep = pow(1. / float(steps), 2.);
		}

		float seed = float(s) + t;
		D = normalize(mix(reflect(D,N), N + normalize(2. * (vec3(hash(seed+p.x), hash(seed+p.y), hash(seed+p.z)) - .5)), 1.-M.specular));
		Lmax = 4.;
	}
#else
	vec3 O = o, D = d;
	vec3 N;
	material_t M;
	float L = 0.;
	int nstep = 0;
	vec3 gic = vec3(0.);
	for (int i = 0; i < 128; ++i) {
		vec3 p = O + D * L;
		float w = world(p);
		if (w < 0.) {
			material_t m = material(p);
			vec3 n = normal(p);
			vec3 c = m.emissive;
			if (nstep == 0) {
				c += enlight(p, n, -D, vec3(5., 5. , 5.), 20.*vec3(1., .3, .2));
				c += enlight(p, n, -D, vec3(8., 8., -8.), 20.*vec3(.2, .8, .3));
				c += enlight(p, n, -D, vec3(-7., 7., 7.), 20.*vec3(.7, .3, .9));
				M = m;
				N = n;
				O = p - D * E * L;
				color = c;
			} else {
				gic += c / max(1., (L*L));
			}

			L = 0.; // TODO preserve L
			float seed = float(i) + t;
			D = normalize(mix(
						reflect(d,N),
						N + normalize(2. * (vec3(hash(p.x), hash(p.y), hash(p.z)) - .5)),
						1.-M.specular));

			//color = vec3(float(i)/256.); // DEBUG
			++nstep;
		} else {
			L += max(w, E * L * .5);
		}
	}

	color += gic / float(nstep);
#endif

	gl_FragColor = vec4(color, 1.);//vec4(pow(color, vec3(1./2.2)), 1.);
}
