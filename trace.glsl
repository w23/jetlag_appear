uniform sampler2D R;
uniform vec3 V, C, A, D, P;

float t = V.z;

const vec3 E = vec3(.0,.001,1.);
const float PI = 3.141593;

/*
vec4 rnd(vec2 n) { return texture2D(R, n/P.xy); }
float hash(float n) { return texture2D(R, vec2(n*17.,n*53.)/P.xy).x; }
float hash(vec2 n) { return texture2D(R, n*17./P.xy).y; }
float noise(vec2 v) { return texture2D(R, (v+.5)/P.xy).z; }
float noise(float v) { return noise(vec2(v)); }
*/

float hash1(float v){return fract(sin(v)*43758.5); }
//float hash2(vec2 p){return fract(1e4*sin(17.*p.x+.1*p.y)*(.1+abs(sin(13.*p.y+p.x))));}
float hash2(vec2 p){return fract(sin(17.*hash1(p.x)+54.*hash1(p.y)));}
//float hash3(vec3 p){return hash2(vec2(hash2(p.xy), p.z));}
float noise2(vec2 p){
	vec2 P=floor(p);p-=P;
	//p*=p*(3.-2.*p);
	return mix(mix(hash2(P), hash2(P+E.zx), p.x), mix(hash2(P+E.xz), hash2(P+E.zz), p.x), p.y);
}
/*float noise3(vec3 p){
	vec3 P=floor(p);p-=P;
	p*=p*(3.-2.*p);
	return mix(
		mix(mix(hash3(P      ), hash3(P+E.zxx), p.x), mix(hash3(P+E.xzx), hash3(P+E.zzx), p.x), p.y),
		mix(mix(hash3(P+E.xxz), hash3(P+E.zxz), p.x), mix(hash3(P+E.xzz), hash3(P+E.zzz), p.x), p.y), p.z);
}
*/
float noise(float p){
	float P = floor(p); p -= P;
	//p*=p*(3.-2.*p);
	return mix(hash1(P), hash1(P+1.), p);
}
vec3 noise13(float p) { return vec3(noise(p), noise(p+13.), noise(p+29.)); }

float fbm(vec2 v) {
	float r = 0.;
	float k = .5;
	for (int i = 0; i < 5; ++i) {
		r += noise2(v) * k;
		k *= .5;
		v *= 2.;
	}
	return r;
}


//float box2(vec2 p, vec2 s) { p = abs(p) - s; return max(p.x, p.y); }
float box3(vec3 p, vec3 s) { p = abs(p) - s; return max(p.x, max(p.y, p.z)); }
//mat3 RX(float a){ float s=sin(a),c=cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
mat3 RY(float a){	float s=sin(a),c=cos(a); return mat3(c,0.,s,0.,1.,0,-s,0.,c); }
//mat3 RZ(float a){ float s=sin(a),c=cos(a); return mat3(c,s,0.,-s,c,0.,0.,0.,1.); }

float ball(vec3 p, float r) { return length(p) - r; }
vec3 rep3(vec3 p, vec3 r) { return mod(p,r) - r*.5; }
//vec2 rep2(vec2 p, vec2 r) { return mod(p,r) - r*.5; }
float ring(vec3 p, float r, float R, float t) {
	float pr = length(p);
	return max(abs(p.y)-t, max(pr - R, r - pr));
}
float vmax(vec2 p) { return max(p.x, p.y); }

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

int mindex = 0;
void PICK(inout float d, float dn, int mat) { if (dn<d) {d = dn; mindex = mat;} }

float world(vec3 p) {
	float w = 1e6;
	PICK(w, bounds(p), 1);
	PICK(w, object(p), 2);
	return w;
}

#if 0
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
#endif

float DistributionGGX(float NH, float r) {
	r *= r; r *= r;
	float denom = NH * NH * (r - 1.) + 1.;
	denom = PI * denom * denom;
	return r / denom;
}
float GeometrySchlickGGX(float NV, float r) {
	r += 1.; r *= r / 8.;
	return NV / (NV * (1. - r) + r);
}

vec3 trace(vec3 o, vec3 d, float maxl) {
	float l = 0., minw = 1e3;
	int i;
	for (i = 0; i < 64; ++i) {
		vec3 p = o+d*l;
		float w = world(p);
		l += w;
		minw = min(minw, w);
		if (w < .002*l || l > maxl) break;
	}
	return vec3(l, minw, float(i));
}

mat3 lookat(vec3 p, vec3 a, vec3 y) {
	vec3 z = normalize(p - a);
	vec3 x = normalize(cross(y, z));
	y = cross(z, x);
	return mat3(x, y, z);
}

vec3 pointlight(vec3 lightpos, vec3 lightcolor, float metallic, float roughness, vec3 albedo, vec3 p, vec3 ray, vec3 normal) {
	vec3 L = lightpos - p; float LL = dot(L,L), Ls = sqrt(LL);
	L = normalize(L);

	vec3 tr = trace(p + .02 * L, L, Ls);
	if (tr.x + .2 < Ls ) return vec3(0.);

	vec3 H = normalize(ray + L), F0 = mix(vec3(.04), albedo, metallic);
	float HV = max(dot(H, ray), .0), NV = max(dot(normal, ray), .0), NL = max(dot(normal, L), 0.), NH = max(dot(normal, H), 0.);
	vec3 F = F0 + (1. - F0) * pow(1. - HV, 5.);
	float G = GeometrySchlickGGX(NV, roughness) * GeometrySchlickGGX(NL, roughness);
	vec3 brdf = DistributionGGX(NH, roughness)* G * F / (4. * NV * NL + .001);
	return 30. * (.7 + .3 * noise(t*20. + lightpos.x)) * ((vec3(1.) - F) * (1. - metallic)* albedo / PI + brdf) * NL * lightcolor / LL;
}

void main() {
	vec2 uv = gl_FragCoord.xy / V.xy * 2. - 1.;
	uv.x *= V.x / V.y;


	vec3 origin = C + .1 * noise13(t*3.);
	mat3 LAT = lookat(origin, A, E.xzx);
	//origin += LAT * vec3(uv*.01, 0.);
	vec3 ray = - LAT * normalize(vec3(uv, -D.y));

	vec3 tr = trace(origin, -ray, 80.);
	vec3 p = origin - tr.x * ray;

	vec3 color = E.xxx, albedo = E.zzz;
	float metallic = 0., roughness = 0.;

	float w=world(p);
	vec3 normal = normalize(vec3(world(p+E.yxx),world(p+E.xyx),world(p+E.xxy))-w);

	if (mindex == 1) {
		float type = smoothstep(.4,.6,fbm(p.xz*4.));
		albedo = vec3(.07);
		roughness = mix(1., .01, type);
		metallic = mix(.01, .9, type);
	} else if (mindex == 2) {
		albedo = vec3(.56, .57, .58);
		roughness = mix(.15, .5, step(box3(rep3(p, vec3(2.)), vec3(.6)), 0.));
		metallic = 0.;

		color = vec3(10.) * smoothstep(.99,.999,sin(t*4.*.4+(rotX(t*.4)*p).y*2.));
		albedo = vec3(.56,.57,.58);
		roughness = .01 + .5 * fbm(p.xy*10.);
		metallic = .5 + fbm(p.zx*21.)*.5;
	} else if (mindex == 3) {
		float stripe = step(0., sin(atan(p.x,p.z)*60.));
		albedo = vec3(mix(1., .1, stripe));
		roughness = mix(.9, .2, stripe);
		metallic = .1;
	} else if (mindex == 4) {
		albedo = vec3(.56, .57, .58);
		roughness = .2 + .6 * pow(noise2(p.xz*4.+40.),4.);
		metallic = .8;
	} else if (mindex == 5) {
		albedo = vec3(1., 0., 0.);
		roughness = .2;
		metallic = .8;
	}


	// vec3 pointlight(vec3 lightpos, vec3 lightcolor, float metallic, float roughness, vec3 albedo, vec3 p, vec3 ray, vec3 normal) {
	color += pointlight(vec3(11., 6.,11.), vec3(.7,.35,.45), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(11., 6.,-11.), vec3(.7,.35,.15), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(-11., 6.,-11.), vec3(.3,.35,.75), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(-11., 6.,11.), vec3(.7,.35,.15), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(0., 10., 0.), vec3(10.), metallic, roughness, albedo, p, ray, normal);


	gl_FragColor = vec4(color, tr.x);
}
