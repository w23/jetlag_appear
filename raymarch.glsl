uniform sampler2D S[8];
uniform float F[32];
vec3 V = vec3(F[0], F[1], F[2]),
		 C = vec3(F[3], F[4], F[5]),
		 A = vec3(F[6], F[7], F[8]),
		 D = vec3(F[9], F[10], F[11]);
float t = F[12] * 20.;

const vec3 E = vec3(.0,.001,1.);
const float PI = 3.141593;

const vec2 P = vec2(256.);
vec4 rnd(vec2 n) { return texture2D(S[0], n/P); }
float noise(float v) { return rnd(vec2(v)).w; }
vec2 noise12(float v) { return rnd(vec2(v)).wx; }
vec3 noise13(float v) { return rnd(vec2(v)).xyz; }
float noise2(vec2 v) { return rnd(v).z; }

//float hash1(float v){return fract(sin(v)*43758.5); }
//float hash2(vec2 p){return fract(1e4*sin(17.*p.x+.1*p.y)*(.1+abs(sin(13.*p.y+p.x))));}
//float hash2(vec2 p){return fract(sin(17.*hash1(p.x)+54.*hash1(p.y)));}
//float hash3(vec3 p){return hash2(vec2(hash2(p.xy), p.z));}

float fbm(vec2 v) {
	float r = 0.;
	float k = .5;
	for (int i = 0; i < 5; ++i) {
		r += noise2(v) * k;
		k *= .5;
		v *= 2.3;
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

float floortype(vec2 p) {
	return smoothstep(.05, .1, pow(fbm(p), 4.));
}

float bounds(vec3 p) {
	float d = box(rep3(p, vec3(4.)), vec3(1.93));
	d = max(d, -box(p+vec3(0., -20., 0.), vec3(19.9)));
	float type = floortype(p.xz);
	//d = mix(d, -box(p+vec3(0.,-20.,0.), vec3(20.)), type);
	d = min(d, -box(p+vec3(0.,-20.,0.), vec3(20.)));
	d -= .015 * floortype(p.xz);
	return d;
}

float object(vec3 p) {
	p -= vec3(0., 3., 0.);

	float cd = dot(p, vec3(1.,0.,0.)*rotY(t));
	const float cr = .4, cs = .07;
	float cut = abs(mod(cd,cr)-cr*.5)-cs;

	//p.x += sin(p.y*2. + t);

	float tt = t * .3;
	p.xy = abs(p.yx); p *= rotX(tt);
	p.xz = abs(p.zx); p *= rotX(tt*.7);
	// p.zy = abs(p.yx); p *= rotY(tt*1.1);
	p.zy = abs(p.yz); p *= rotY(tt*1.1);

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

#if 0
	vec3 tr = trace(p + .02 * L, L, Ls);
	if (tr.x < Ls ) return vec3(0.);
	float kao = 1.;//smoothstep(.00, .05, tr.y);
#elif 1
	float kao = 1.;
	const int Nao = 15;
	float ki = 1. / float(Nao);
	for (int i = 0; i < Nao; ++i) {
		float dist = .25 * ki * float(i);
		vec3 p = p + (.03 + dist) * L;//normal;
		kao -= ki * smoothstep(-.1, .1, dist - world(p));
	}
#else
	const float kao = 1.;
#endif

	vec3 H = normalize(ray + L), F0 = mix(vec3(.04), albedo, metallic);
	float HV = max(dot(H, ray), .0), NV = max(dot(normal, ray), .0), NL = max(dot(normal, L), 0.), NH = max(dot(normal, H), 0.);
	vec3 F = F0 + (1. - F0) * pow(1. - HV, 5.);
	float G = GeometrySchlickGGX(NV, roughness) * GeometrySchlickGGX(NL, roughness);
	vec3 brdf = DistributionGGX(NH, roughness)* G * F / (4. * NV * NL + .001);
	return /*30. * (.7 + .3 * noise(t*20. + lightpos.x)) */ kao * ((vec3(1.) - F) * (1. - metallic)* albedo / PI + brdf) * NL * lightcolor / LL;
}

void main() {
	vec2 uv = gl_FragCoord.xy / V.xy * 2. - 1.;
	uv.x *= V.x / V.y;


	vec3 origin = 10. * (vec3(F[27],F[28],F[29]) - .5);// + .1 * noise13(t*3.);
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
		//float type = smoothstep(.2, 0., p.y);
		float type = floortype(p.xz);//smoothstep(.4,.6,fbm(p.xz*(8.*F[17])));
		albedo = vec3(.07);
		roughness = mix(.9, .1, type);
		metallic = mix(0., .99, type);
	} else if (mindex == 2) {
		albedo = vec3(.56, .57, .58);
		roughness = F[26];//mix(.15, .5, step(box3(rep3(p, vec3(2.)), vec3(.6)), 0.));
		metallic = F[25];
		//color = vec3(10.) * smoothstep(.96,.999,sin(t*4.*.4+(rotX(t*.4)*p).y*6.));
		//albedo = vec3(.56,.57,.58);
		//roughness = .01 + .5 * fbm(p.xy*10.);
		//metallic = .5 + fbm(p.zx*21.)*.5;
	} else if (mindex == 3) {
		float stripe = step(0., sin(atan(p.x,p.z)*60.));
		albedo = vec3(mix(1., .0, stripe));
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
	/*
	color += pointlight(vec3(11., 6.,11.), vec3(.7,.35,.45), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(11., 6.,-11.), vec3(.7,.35,.15), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(-11., 6.,-11.), vec3(.3,.35,.75), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(-11., 6.,11.), vec3(.7,.35,.15), metallic, roughness, albedo, p, ray, normal);
	*/
	color += pointlight(vec3(4., 4., 0.), 1000.*F[16]*vec3(F[13],F[14],F[15]), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(10.*(vec3(F[18], F[19], F[20]) - .5), 1000.*F[21]*vec3(F[22],F[23],F[24]), metallic, roughness, albedo, p, ray, normal);


	gl_FragColor = vec4(color, tr.x);
}
