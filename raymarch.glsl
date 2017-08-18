vec4 rnd(vec2 n) { return T(0,n-.5); }//texture2DLod(S[0], (n-.5)/textureSize(S[0],0), 0); }
//float noise(float v) { return rnd(vec2(v)).w; }
//vec2 noise12(float v) { return rnd(vec2(v)).wx; }
vec3 noise13(float v) { return rnd(vec2(v)).xyz; }
//float noise2s(vec2 v) { vec2 V = floor(v); v-=V; v*=v*(3.-2.*v); return rnd(V + v).z; }
float noise21(vec2 v) { return rnd(v).z; }
//vec2 noise22(vec2 v) { return rnd(v).wx; }
float noise31(vec3 v) { return .5 * (rnd(v.yz).x+rnd(v.xy).x); }

//float hash1(float v){return fract(sin(v)*43758.5); }
//float hash2(vec2 p){return fract(1e4*sin(17.*p.x+.1*p.y)*(.1+abs(sin(13.*p.y+p.x))));}
//float hash2(vec2 p){return fract(sin(17.*hash1(p.x)+54.*hash1(p.y)));}
//float hash3(vec3 p){return hash2(vec2(hash2(p.xy), p.z));}

/*
float fbm(vec2 v) {
	float r = 0.;
	float k = .5;
	for (int i = 0; i < 5; ++i) {
		r += noise21(v) * k;
		k *= .5;
		v *= 2.3;
	}
	return r;
}*/

//float box2(vec2 p, vec2 s) { p = abs(p) - s; return max(p.x, p.y); }
//float box3(vec3 p, vec3 s) { p = abs(p) - s; return max(p.x, max(p.y, p.z)); }
mat3 RX(float a){ float s=sin(a),c=cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
mat3 RY(float a){	float s=sin(a),c=cos(a); return mat3(c,0.,s,0.,1.,0,-s,0.,c); }
//mat3 RZ(float a){ float s=sin(a),c=cos(a); return mat3(c,s,0.,-s,c,0.,0.,0.,1.); }

//float ball(vec3 p, float r) { return length(p) - r; }
vec3 rep3(vec3 p, vec3 r) { return mod(p,r) - r*.5; }
vec2 rep2(vec2 p, vec2 r) { return mod(p,r) - r*.5; }
float ring(vec3 p, float r, float R, float t) {
	return max(abs(p.y)-t, max(length(p) - R, r - length(p)));
}
//float vmax(vec2 p) { return max(p.x, p.y); }
float vmax(vec3 v) { return max(v.x, max(v.y, v.z)); }
float box(vec3 p, vec3 s) { return vmax(abs(p) - s);}
float tor(vec3 p, vec2 s) {
	return length(vec2(length(p.xz) - s.x, p.y)) - s.y;
}

struct {
	vec4 posr;
	vec3 color;
} sphere_light;

struct {
	vec4 posr;
	vec3 color;
} tube_light;

float ground(vec3 p) {
	return p.y + 3.;
}

float room(vec3 p) {
	p.y -= 3.;
	float d = -box(p, vec3(10., 3., 20.));
	return d;
}

/*
float dbgNoise(vec3 p) {
	return max(length(p) - 5., noise31(p) - .5);
}
*/

/*
float grid(vec3 p) {
	const vec2 ls = vec2(8.,.4);
	p*=RX(.3);
	float d = box(p, ls.xxx*1.5);
	p = rep3(p, ls.xxx);
	return max(-d,
			min(box(p, ls.xyy),min(box(p, ls.yxy),box(p, ls.yyx))));
}
*/

float object(vec3 p) {
	return max(box(p, vec3(10., 2., 10.)), length(rep3(p-2.,vec3(4.))) - 1.5);
}
int mindex;
void PICK(inout float d, float dn, int mat) { if (dn<d) {d = dn; mindex = mat;} }

float world(vec3 p) {
	mindex = 2;
	float w = room(p);
	//PICK(w, object(p), 10);
	PICK(w, length(p-sphere_light.posr.xyz) - sphere_light.posr.w, 100);
	return w;
}

float world_nolights(vec3 p) {
	mindex = 2;
	float w = room(p);
	//PICK(w, object(p), 10);
	return w;
}

float trace(vec3 o, vec3 d) {
	float l = 0., w;
	int i;
	for (i = 0; i < 64; ++i) {
		w = world(o + d * l);
		l += w;//*.2;//mix(1.,.2,step(length(p),1.));
		if (w < .001*l) break;
	}
	return l;
}

vec3 ray_pos, ray, normal;
vec3 color, albedo;
float metallic, roughness;

/*
float DistributionGGX(float NH, float r) {
	r *= r; r *= r;
	float denom = NH * NH * (r - 1.) + 1.;
	denom = PI * denom * denom;
	return r / max(1e-10, denom);
}
*/

float GeometrySchlickGGX(float NV, float r) {
	r += 1.; r *= r / 8.;
	return NV / (NV * (1. - r) + r);
}
vec3 pbr(vec3 lightdir, float ap) {
	roughness = max(.01, roughness);
	vec3 H = normalize(ray + lightdir), F0 = mix(vec3(.04), albedo, metallic);
	float HV = max(dot(H, ray), .0),
				NV = max(dot(normal, ray), .0),
				NL = max(dot(normal, lightdir), 0.),
				NH = max(dot(normal, H), 0.),
				r4 = roughness * roughness * roughness * roughness,
				GGXdenom = NH * NH * (r4 - 1.) + 1.;
	vec3 F = F0 + (1. - F0) * pow(1.001 - HV, 5.);
	float G = GeometrySchlickGGX(NV, roughness) * GeometrySchlickGGX(NL, roughness);
	//vec3 brdf = DistributionGGX(NH, roughness) * G * F / max(1e-10, 4. * NV * NL);
	vec3 brdf = roughness * roughness * ap * ap / max(1e-10, PI * GGXdenom * GGXdenom) * G * F / max(1e-10, 4. * NV * NL);
	return ((E.zzz - F) * (1. - metallic) * albedo / PI + brdf) * NL;
}

vec3 dirlight(vec3 L, float Ls, float LL, vec3 lightcolor, float ap) {
	float kao = 1.;
	/*
	const int Nao = 8;//16
	float d, ki = 1. / float(Nao);
	for (int i = 1; i < Nao; ++i) {
		d = min(Ls, 2.) * float(i) * ki;
		vec3 pp = ray_pos + d * L;
		d = min(1., world_nolights(pp)/(d * 1.));
		kao = min(kao, d * d);
	}
	*/

	//return vec3(kao);
	return pbr(L, ap) * lightcolor * kao / LL;
}

vec3 pointlight(vec3 lightpos, vec3 lightcolor) {
	vec3 L = lightpos - ray_pos;
	float LL = dot(L,L), Ls = sqrt(LL);
	L /= Ls;

	return dirlight(L, Ls, LL, lightcolor, roughness * roughness);
}


vec3 spherelight(vec4 posr, vec3 lightcolor) {
	vec3 refl = reflect(ray, normal),
			 L = posr.xyz - ray_pos,
			 l = dot(L, refl) * refl - L;
	l = L + l * clamp(posr.w/length(l), 0., 1.);

	return dirlight(normalize(l), dot(l,l), length(l), lightcolor,
		clamp(posr.w / (2. * length(L)) + roughness * roughness, 0., 1.));
}

vec4 raycast() {
	float tr = trace(ray_pos, -ray);
	ray_pos -= tr * ray;

	color = E.xxx;

	// mindex == 0: floor
	metallic = roughness = 0.;
	albedo = vec3(.03,.08,.07);
	roughness = mix(.1,.9, mod(floor(ray_pos.x*.5)+floor(ray_pos.z*.5),2.));

	float w=world(ray_pos);
	normal = normalize(vec3(world(ray_pos+E.yxx),world(ray_pos+E.xyx),world(ray_pos+E.xxy))-w);

	// mindex == 1: object
	// debug
	if (mindex == 100) {
		albedo = E.xxx;
		color = sphere_light.color;
		roughness = 0.;
	}
	if (mindex == 10) {
		vec2 pp = floor((ray_pos.xz+10.) / 4.) / 5.;
		roughness = pp.x;
		metallic = pp.y;
		albedo = vec3(1.);
	}
	if (mindex == 2) { // mindex == 2: room/walls
		albedo = vec3(.56, .57, .58);
		roughness = .55;//F[14];//mix(.15, .5, step(box3(rep3(ray_pos, vec3(2.)), vec3(.6)), 0.));
		metallic = 1.;//F[15];

		// stripes
		//color = vec3(2.) * smoothstep(.99,.999,sin(t*8.+(RX(t*.4)*ray_pos).y*2.));
		//albedo = vec3(.56,.57,.58);
		//roughness = .01 + .5 * fbm(ray_pos.xy*10.);
		//metallic = .5 + fbm(ray_pos.zx*21.)*.5;
	}
	/*
	color += pointlight(.5*vec3(-20.,10.,-30.), 50.*vec3(.9,.8,.7));
	color += pointlight(vec3( 20.,10.,-30.), 50.*vec3(.9,.8,.7));
	color += pointlight(vec3( 20.,10., 30.), 50.*vec3(.9,.8,.7));
	color += pointlight(vec3(-20.,10., 30.), 50.*vec3(.9,.8,.7));
	*/

	color += spherelight(sphere_light.posr, sphere_light.color);
	//color += pointlight(sphere_light.posr.xyz, sphere_light.color);

	/*
	color += pointlight(vec3(3.,7.,-3.), 1000.*vec3(.2,.5,.9));
	color += pointlight(vec3(3.,6.,3.), 500.*vec3(.5,.9,.2));
	color += pointlight(vec3(-3.,5.,3.), 500.*vec3(.9,.5,.2));
	//color += pointlight(vec3(0.,6.,0.), vec3(100.));
	*/

	return vec4(color, tr);
}

mat3 lookat(vec3 p, vec3 a, vec3 y) {
	vec3 z = normalize(p - a);
	vec3 x = normalize(cross(y, z));
	y = cross(z, x);
	return mat3(x, y, z);
}
vec3 cyl(vec3 p) {
	return vec3(p.x*cos(p.z*PI2), p.y, p.x*sin(p.z*PI2));
}
void main() {
	vec2 V = Z(1);
	vec2 uv = X / V * 2. - 1.;
	uv.x *= V.x / V.y;

	sphere_light.posr = vec4(6.*sin(t), 3., 6.*cos(t), .5);
	sphere_light.color = vec3(10.);

//	gl_FragData[0] = gl_FragData[1] = vec4(uv, sin(F[0]), 0.); return;

	float tl = t * .6;
	float tt = tl;//floor(tl) + pow(fract(tl), 4.);

	vec3 origin = vec3(F[2], F[3], F[4]);//mix(rnd(vec2(floor(tl)*3.)), rnd(vec2(floor(tl)*3.+1.)), fract(tl)).xzy;

	//vec3 origin = 30. * (vec3(F[27],F[28]+.501,F[29]) - .5);// + .1 * noise13(t*3.);
	origin = cyl(origin) + .9*noise13(t);
	mat3 LAT = lookat(origin, vec3(0.,0.,0.)+noise13(tt), E.xzx);
	//origin += LAT * vec3(uv*.01, 0.);
	ray = - LAT * normalize(vec3(uv, -1.));//-D.y));

	//gl_FragData[0] = vec4(ray,0.); return;

	//vec3 p, n;
	ray_pos = origin;
	gl_FragData[0] = raycast();
	gl_FragData[1] = vec4(albedo, roughness);
	ray = reflect(ray, normal);
	//ray_pos -= ray * .02;
	gl_FragData[1].xyz *= raycast().xyz;

	//TODO IBL
	//gl_FragData[1].xyz = pbr(ray2) * raycast().xyz;
}
