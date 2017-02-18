uniform float uf_time;
uniform vec2 uv2_resolution;
varying vec2 vv2_pos;

float t = uf_time;

float hash(float n){return fract(sin(n) * 43758.5453123);}
float hash(vec2 co){
	    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
//float hash(vec2 f){return hash(dot(f,vec2(., 137.))); }
float noise(float v) {
	float F = floor(v), f = fract(v); return mix(hash(F), hash(F+1.), f);
}
float noise(vec2 v) {
	vec2 e = vec2(1.,0.);
	vec2 F = floor(v), f = fract(v);
	f *= f * (3. - 2. * f);
	return mix(mix(hash(F), hash(F+e.xy), f.x), mix(hash(F+e.yx), hash(F+e.xx), f.x), f.y);
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
	return -box(p+vec3(0.,-10.,0.), vec3(10.));
}

float object(vec3 p) {
	p -= vec3(0., 3., 0.);

	//p.x += sin(p.y*2. + t);

#if 1
	p.xy = abs(p.yx); p *= rotX(t);
	p.xz = abs(p.zx); p *= rotX(t*.7);
	// INTERESTING p.zy = abs(p.yx); p *= rotY(t*1.1);
	p.zy = abs(p.yz); p *= rotY(t*1.1);
#endif

	return min(min(min(
			box(p, vec3(.7,2.4,.2)),
			box(p, vec3(1.3,.4,.3))),
			box(rotX(.6)*p, vec3(.3,.4,2.3))),
			tor(p, vec2(1., .1))
			);
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
	if (object(p) < bounds(p)) {
		m.emissive = vec3(2.) * smoothstep(.99,.999,sin(t*4.+(rotX(t)*p).y*7.));
		m.color = vec3(.56,.57,.58);
		m.roughness = .01;
		m.specular = .8;
		m.fresnel = 0.;
		m.metalness = 1.;
	} else {
		float type = smoothstep(.4,.6,noise(p.xz*4.));
		m.emissive = vec3(0.);
		m.color = vec3(.07);
		m.roughness = mix(1., .01, type);
		m.specular = mix(.1, .9, type);
		m.fresnel = .02;
		m.metalness = mix(.01, .9, type);
	}
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
	float md = Lmax;
	for (int i = 0; i < 128; ++i) {
		vec3 p = O + D * L;
		float d = world(p);
		md = min(md, d);
		if (d < 0.) return vec3(Lp, L, md);
		Lp = L;
		L += max(d, E * L * .5);
		if (L >= Lmax) return vec3(Lmax, Lmax, md);
	}
	return vec3(-1., L, md);
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
	vec3 H = (L + V) / length(L + V);
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

vec3 enlight(vec3 pos, vec3 normal, vec3 view, vec3 diffuse, vec3 lightpos, vec3 lightcolor) {
	vec3 lightdir = lightpos - pos;
	vec3 nld = normalize(lightdir);
	float ldist = length(lightdir);
	float d = trace(pos + normal*E*10., nld, E, ldist).z;
	material_t m = material(pos);
	return m.emissive + pow(smoothstep(0., .06, d), 2.) * brdf(normal,nld,view,m) * lightcolor / dot(lightdir, lightdir);
}

vec3 directlight(vec3 p, vec3 v) {
		vec3 n = normal(p);
		material_t m = material(p);
		vec3 color = vec3(.0);
		color += enlight(p, n, v, vec3(1.), vec3(5., 5. , 5.), 20.*vec3(1., .3, .2));
		color += enlight(p, n, v, vec3(1.), vec3(8., 8., -8.), 20.*vec3(.2, .8, .3));
		color += enlight(p, n, v, vec3(1.), vec3(-7., 7., 7.), 20.*vec3(.7, .3, .9));
		return color;
}

void main() {
	//t = uf_time * .1;
	vec2 uv = vv2_pos * vec2(1., uv2_resolution.y / uv2_resolution.x);

	//gl_FragColor = vec4(noise(gl_FragCoord.x*.1)); return;
	//gl_FragColor = vec4(noise(uv*10.)); return;

	vec3 color = vec3(0.);

	vec3 at = vec3(2.2,3.,0.);
	vec3 eye = vec3(3.7, 4.5, 4.7) + .1 * vec3(noise(t), noise(t*.4), noise(t*.7));
	vec3 up = vec3(0., 1., 0.);
	vec3 dir = normalize(at - eye);
	vec3 side = cross(up, dir); up = cross(dir, side);
	mat3 lat = mat3(side, up, -dir);
	
	vec3 o = vec3(uv, 0.);
	vec3 d = normalize(vec3(uv, -1.));

	o = lat * o + eye;
	d = lat * d;

	float Lmax = 30.;
	vec3 ll = trace(o, d, 0., Lmax);
	if (ll.x > 0. && ll.x < Lmax) {
		vec3 p = refine(o + d * ll.x, o + d * ll.y);
		vec3 n = normal(p);
		material_t m = material(p);
		color = directlight(p, -d);
		int steps = 8;
		float ksteps = pow(1. / float(steps), 2.);

		for (int i = 0; i < steps; ++i) {
			float seed = float(i) + t;
			//vec3 sd = normalize(mix(reflect(d,n), normalize(2. * (vec3(hash(seed+p.x), hash(seed+p.y), hash(seed+p.z)) - .5)), .5));
			//vec3 sd = reflect(d, n);
			vec3 sd = normalize(mix(reflect(d,n), n + normalize(2. * (vec3(hash(seed+p.x), hash(seed+p.y), hash(seed+p.z)) - .5)), 1.-m.specular));
			vec3 os = p + n * E * 2.;
			float Lm = 4.;
			vec3 l = trace(os, sd, 0., Lm);
			if (l.x > 0. && l.x < Lm) {
				//vec3 ps = refine(os + sd * l.x, os + sd * l.y);
				vec3 ps = os + sd * l.x;
				color += /* brdf(n, sd, -d, m) */  directlight(ps, -sd)
					* ksteps  // l.x; // float(steps);
				;
			}
		}
	}

	gl_FragColor = vec4(pow(color, vec3(1./2.2)), 1.);
}
