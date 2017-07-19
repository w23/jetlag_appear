out vec4 C0, C1, C2;
vec4 rnd(vec2 n) { return texture2DLod(S[0], (n-.5)/textureSize(S[0],0), 0); }
float noise(float v) { return rnd(vec2(v)).w; }
vec2 noise12(float v) { return rnd(vec2(v)).wx; }
vec3 noise13(float v) { return rnd(vec2(v)).xyz; }
float noise2s(vec2 v) { vec2 V = floor(v); v-=V;
	v*=v*(3.-2.*v);
	return rnd(V + v).z; }
float noise2(vec2 v) { return rnd(v).z; }
vec2 noise22(vec2 v) { return rnd(v).wx; }
float noise31(vec3 v) { return .5 * (rnd(v.yz).x+rnd(v.xy).x); }

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
mat3 RX(float a){ float s=sin(a),c=cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
mat3 RY(float a){	float s=sin(a),c=cos(a); return mat3(c,0.,s,0.,1.,0,-s,0.,c); }
//mat3 RZ(float a){ float s=sin(a),c=cos(a); return mat3(c,s,0.,-s,c,0.,0.,0.,1.); }

float ball(vec3 p, float r) { return length(p) - r; }
vec3 rep3(vec3 p, vec3 r) { return mod(p,r) - r*.5; }
vec2 rep2(vec2 p, vec2 r) { return mod(p,r) - r*.5; }
float ring(vec3 p, float r, float R, float t) {
	return max(abs(p.y)-t, max(length(p) - R, r - length(p)));
}
float vmax(vec2 p) { return max(p.x, p.y); }
float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }
float box(vec3 p, vec3 s) { return vmax(abs(p) - s);}
float tor(vec3 p, vec2 s) {
	return length(vec2(length(p.xz) - s.x, p.y)) - s.y;
}

vec3 rep(vec3 p, vec3 x) { return mod(p, x) - x*.5; }

float groundHeight(vec3 p) {
	//return /*.01 * (1.-pow(noise2(p.xz*50.),3.)) + */ .05 * (noise2(p.xz*.5) - .5);
	return .05 * (1.-pow(noise2(p.xz*20.),3.)) + .3 * (noise2(p.xz*.6) - .3);
}

float floor(vec3 p) {
	float tt = t * 4.; 
	return p.y - max(0., groundHeight(p));
}

float room(vec3 p) {
	vec3 po = p;
	p.y += 60.;
	float thick = .4;
	float a = atan(p.x, p.y)*60.;
	float r = length(p.xy);
	float z = mod(p.z,10.)-5.;
	float d = max(max(77.-r,r-79.), abs(z)-thick*.5);
	vec3 pr = vec3(mod(a,3.)-1.5,r-78.,p.z);
	d = max(d, .8-length(pr.xy));
	//d += .2*step(length(pr.xy), .8);
	d = min(d, 80. - r);
	d = min(d, box(pr-E.xzx*2., vec3(.1,.1,1000.)));
	d = min(d, - abs(p.x) + 25);
	d = min(d, - abs(p.z) + 50);
	p.x -= 12.5;
	p.xz = rep2(p.xz, vec2(25., 10.));
	p.y -= 60.;
	d = min(d, box(p - E.xzx*13., vec3(thick, thick, 10.)));
	d = min(d, box(p, vec3(thick, 17., thick)));

	if (d < .1)
		d += .04 * noise31(po*10.) - .1 * step(noise31(po*4.),.7);
	return d;
}

float dbgNoise(vec3 p) {
	return max(length(p) - 5., noise31(p) - .5);
}

float grid(vec3 p) {
	const vec2 ls = vec2(8.,.4);
	p*=RX(.3);
	float d = box(p, ls.xxx*1.5);
	p = rep3(p, ls.xxx);
	return max(-d,
			min(box(p, ls.xyy),min(box(p, ls.yxy),box(p, ls.yyx))));
}

float object(vec3 p) {
	p -= vec3(0., 3., 0.);

	float cd = dot(p, vec3(1.,0.,0.)*RY(t));
	const float cr = .4, cs = .07;
	float cut = abs(mod(cd,cr)-cr*.5)-cs;

	//p.x += sin(p.y*2. + t);

#if 1
	float tt = t * .3;
	p.xy = abs(p.yx); p *= RX(tt);
	p.xz = abs(p.zx); p *= RX(tt*.7);
	// p.zy = abs(p.yx); p *= RY(tt*1.1);
	p.zy = abs(p.yz); p *= RY(tt*1.1);
#endif

	float d = min(min(min(
			box(p, vec3(.7,2.4,.2)),
			box(p, vec3(1.3,.4,.3))),
			box(RX(.6)*p, vec3(.3,.4,2.3))),
			tor(p, vec2(1., .1))
			);

	return max(d, -cut);
}

int mindex = 0;
void PICK(inout float d, float dn, int mat) { if (dn<d) {d = dn; mindex = mat;} }

float world(vec3 p) {
	float w = 1e6;
	PICK(w, floor(p), 1);
	PICK(w, room(p), 3);
	//PICK(w, dbgNoise(p), 3);
	PICK(w, object(p), 2);
	//PICK(w, grid(p), 3);
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

vec3 trace(vec3 o, vec3 d) {
	float l = 0., minw = 1e3;
	int i;
	for (i = 0; i < 64; ++i) {
		vec3 p = o+d*l;
		float w = world(p);
		l += w;
		minw = min(minw, w);
		if (w < .002*l) break;
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
	vec3 tr = trace(p + .02 * L, L);
	if (tr.x < Ls ) return vec3(0.);
	float kao = 1.;//smoothstep(.00, .05, tr.y);
#elif 1
	float kao = 1.;
	const int Nao = 15;
	float ki = 1. / float(Nao);
	for (int i = 0; i < Nao; ++i) {
		float dist = 2. * pow(ki * float(i), 1.3);
		vec3 p = p + (.03 + dist) * L;//normal;
		kao -= ki * smoothstep(-.05, .05, dist - world(p));
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

vec3 color = E.xxx, albedo = E.zzz;
float metallic = 0.;

vec4 raycast(vec3 origin, vec3 ray, out vec3 p, out vec3 normal, out float roughness) {
	vec3 tr = trace(origin, -ray);
	p = origin - tr.x * ray;

	color = E.xxx, albedo = E.zzz; metallic = roughness = 0.;

	float w=world(p);
	normal = normalize(vec3(world(p+E.yxx),world(p+E.xyx),world(p+E.xxy))-w);

	if (mindex == 1) {
		//float type = smoothstep(.2, 0., p.y);
		//float type = step(groundHeight(p), .01*fbm(p.xz*9.));//smoothstep(.4,.6,fbm(p.xz*(8.*F[17])));
		float type = step(groundHeight(p), .001);// .01*fbm(p.xz*9.));//smoothstep(.4,.6,fbm(p.xz*(8.*F[17])));
		albedo = mix(vec3(.12), vec3(.03,.08,.07), type);
		roughness = mix(.99, .05, type);
		//metallic = mix(0., .1, type);
		normal.xz += .002 * type * (
			sin(t*4. + 6.*noise22(10.*p.xz)) + 
			.6 * sin(t*2. + 6.*noise22(15.*p.xz)) +
			.3 * sin(t*8. + 6.*noise22(25.*p.xz)));
		normal = normalize(normal);
	} else if (mindex == 2) {
		albedo = vec3(.56, .57, .58);
		roughness = F[26];//mix(.15, .5, step(box3(rep3(p, vec3(2.)), vec3(.6)), 0.));
		metallic = F[25];
		//color = vec3(10.) * smoothstep(.96,.999,sin(t*4.*.4+(RX(t*.4)*p).y*6.));
		//albedo = vec3(.56,.57,.58);
		//roughness = .01 + .5 * fbm(p.xy*10.);
		//metallic = .5 + fbm(p.zx*21.)*.5;
	} else if (mindex == 3) {
#if 0
		float stripe = step(0., sin(atan(p.x,p.z)*60.));
		albedo = vec3(mix(1., .0, stripe));
		roughness = mix(.9, .2, stripe);
		metallic = .1;
#else
		float type = step(noise31(p*4.), .7);
		albedo = mix(vec3(0.662124, 0.654864, 0.633732), vec3(.7,.66,.6), type);
		roughness = mix(1.,.7, type);
		metallic = mix(0.,.4,type);
#endif
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
	color += pointlight(vec3(11., 6.,11.), 100.*vec3(1.), metallic, roughness, albedo, p, ray, normal);
#if 0
	color += pointlight(vec3(11., 6.,11.), 100.*vec3(.7,.35,.45), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(11., 6.,-11.),100.* vec3(.7,.35,.15), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(-11., 6.,-11.), 100.*vec3(.3,.35,.75), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(-11., 6.,11.), 100.*vec3(.7,.35,.15), metallic, roughness, albedo, p, ray, normal);
#endif
	//color += pointlight(vec3(4., 4., 0.), 1000.*F[16]*vec3(F[13],F[14],F[15]), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(2., 6. + 3.*sin(t), 0.), 100.*vec3(.85,.43,.56), metallic, roughness, albedo, p, ray, normal);
	//color += pointlight(10.*(vec3(F[18], F[19], F[20]) - .5), 1000.*F[21]*vec3(F[22],F[23],F[24]), metallic, roughness, albedo, p, ray, normal);
	color += pointlight(vec3(-1.,4.,2.), 100.*vec3(.14,.33,.23), metallic, roughness, albedo, p, ray, normal);

	return vec4(color, tr.x);
}

void main() {
	vec2 uv = gl_FragCoord.xy / V.xy * 2. - 1.;
	uv.x *= V.x / V.y;

	vec3 origin = 30. * (vec3(F[27],F[28]+.501,F[29]) - .5);// + .1 * noise13(t*3.);
	mat3 LAT = lookat(origin, A, E.xzx);
	//origin += LAT * vec3(uv*.01, 0.);
	vec3 ray = LAT * normalize(vec3(uv, -D.y));

	vec3 p, n;
	float r1, r2;
	C0 = raycast(origin, -ray, p, n, r1);
	vec3 ray2 = reflect(ray, n);
	C2 = vec4(1.);//vec4(pointlight(p + ray2, E.zzz, metallic, r1, albedo, p, -ray, n), 0.);
	C1 = raycast(p + ray2 + .02, -ray2, p, n, r2);
	C1.w = r1;
	//c2.w += c1.w;

	//gl_FragColor = mix(c1, c2, pow(1.-r1,2.));
	//gl_FragColor = c1;
	//gl_FragData[0] = c1;
	//gl_FragData[1] = c2;
}
