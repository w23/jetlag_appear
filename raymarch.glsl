vec4 rnd(vec2 n) { return T(0,n-.5); }//texture2DLod(S[0], (n-.5)/textureSize(S[0],0), 0); }
//float noise(float v) { return rnd(vec2(v)).w; }
//vec2 noise12(float v) { return rnd(vec2(v)).wx; }
//vec3 noise13(float v) { return rnd(vec2(v)).xyz; }
//float noise2s(vec2 v) { vec2 V = floor(v); v-=V; v*=v*(3.-2.*v); return rnd(V + v).z; }
//float noise21(vec2 v) { return rnd(v).z; }
//vec2 noise22(vec2 v) { return rnd(v).wx; }
vec4 noise34(vec3 v) { return .5 * (rnd(v.yz)+rnd(v.zx)); }
//float noise31(vec3 v) { return .5 * (rnd(v.yz).x+rnd(v.zx).z); }
//float noise31(vec3 v) { return (rnd(vec2(v.x)) + rnd(vec2(v.y)) + rnd(vec2(v.z))).y / 3.; }

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
//mat3 RX(float a){ float s=sin(a),c=cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
//mat3 RY(float a){	float s=sin(a),c=cos(a); return mat3(c,0.,s,0.,1.,0,-s,0.,c); }
mat3 RZ(float a){ float s=sin(a),c=cos(a); return mat3(c,s,0.,-s,c,0.,0.,0.,1.); }

//float ball(vec3 p, float r) { return length(p) - r; }
vec3 rep3(vec3 p, vec3 r) { return mod(p,r) - r*.5; }
//vec2 rep2(vec2 p, vec2 r) { return mod(p,r) - r*.5; }
/*float ring(vec3 p, float r, float R, float t) {
	return max(abs(p.y)-t, max(length(p) - R, r - length(p)));
}*/
//float vmax2(vec2 p) { return max(p.x, p.y); }
float vmax3(vec3 v) { return max(v.x, max(v.y, v.z)); }
float box(vec3 p, vec3 s) { return vmax3(abs(p) - s);}
//float tor(vec3 p, vec2 s) { return length(vec2(length(p.xz) - s.x, p.y)) - s.y; }

/*
struct {
	vec4 posr;
	vec3 color;
} tube_light;
*/

float techfld(vec3 p) {
	//p=RX(t*.1)*p;
	//p.y+=t*.5;
	float
	d = box(rep3(p,vec3(2.)), vec3(.8));
	d = max(d, -box(rep3(p,vec3(2.)), vec3(.6)));
	d = max(d, -box(rep3(p,vec3(1.3)), vec3(.5)));

	//d = max(d, -box(p, vec3(10., 3., 20.)+vec3(.1)));
	//d = max(d, -box(rep3(p,vec3(4.)), vec3(1.4)));
	//d = min(d, -box(p, vec3(10., 3., 20.)+vec3(.2)));

	d = max(d, -box(rep3(p,vec3(.3)), vec3(.11)));
	return d;
}

vec2 room(vec3 p) {
	p.y -= 3.;
	float d = -box(p, vec3(8., 3., 14.));
	//return d;
	if (d < .1)
	{
		//p.x+=.3;//+2.*sin(t);
		d = max(d, min(d+.2, techfld(RZ(.7)*p*2.)*.5));
		//d = max(d, max(d+.2, -techfld(RZ(.7)*p*2.)*.5));
	}
	return vec2(d, 0.);
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

vec2 object(vec3 p) {
	//return max(box(p, vec3(10., 2., 10.)), length(rep3(p-2.,vec3(4.))) - 1.5);
	return vec2(length(p.xz)-.2, 1.);
}

//int mindex;
//void PICK(inout float d, float dn, int mat) { if (dn<d) {d = dn; mindex = mat;} }

vec4 light0_pos;
vec3 light0_col;
int dist_lights = 1;
vec2 world(vec3 p) {
	//mindex = 0;
	vec2 w = room(p), d;
	d = object(p);
	w = w.x < d.x ? w : d;
	if (dist_lights != 0) {
		d = vec2(length(p-light0_pos.xyz) - light0_pos.w, 100.);
		w = w.x < d.x ? w : d;
	}
	return w;
}

vec2 trace(vec3 o, vec3 d) {
	float l = 0.;
	vec2 w;
	for (int i = 0; i < 64; ++i) {
		w = world(o + d * l);
		l += w.x;//*.2;//mix(1.,.2,step(length(p),1.));
		if (w.x < .001*l) break;
	}
	return vec2(l, w.y);
}

vec3 ray_pos, ray, normal;
vec3 color, albedo, F0;
float metallic, roughness;

/*
float DistributionGGX(float NH, float r) {
	r *= r; r *= r;
	float denom = NH * NH * (r - 1.) + 1.;
	denom = PI * denom * denom;
	return r / max(1e-10, denom);
}
*/

float alpha, alphaP;

float GeometrySchlickGGX(float NV) {
	roughness += 1.; roughness *= roughness / 8.;
	return NV / (NV * (1. - roughness) + roughness);
}
vec3 pbr(vec3 lightdir) {
	//roughness = max(.01, roughness);
	vec3 H = normalize(ray + lightdir);
	F0 = mix(vec3(.04), albedo, metallic);
	float HV = max(dot(H, ray), .0),
				NV = max(dot(normal, ray), .0),
				NL = max(dot(normal, lightdir), 0.),
				NH = max(dot(normal, H), 0.),
				r4 = roughness * roughness * roughness * roughness,
				GGXdenom = NH * NH * (r4 - 1.) + 1.;
	vec3 F = F0 + (1. - F0) * pow(1.001 - HV, 5.);
	float G = GeometrySchlickGGX(NV) * GeometrySchlickGGX(NL);
	//vec3 brdf = DistributionGGX(NH, roughness) * G * F / max(1e-10, 4. * NV * NL);
	//vec3 brdf = r4 / max(1e-10, PI * GGXdenom * GGXdenom) * G * F / max(1e-10, 4. * NV * NL);
	vec3 brdf = alpha * alpha * alphaP * alphaP / max(1e-10, PI * GGXdenom * GGXdenom) * G * F / max(1e-10, 4. * NV * NL);
	return ((E.zzz - F) * (1. - metallic) * albedo / PI + brdf) * NL;
}

vec3 dirlight(vec3 L, float Ls, float LL) {
	float kao = 1.;
	const int Nao = 8;//16
	float d, ki = 1. / float(Nao);
	for (int i = 1; i < Nao; ++i) {
		d = min(Ls, 2.) * float(i) * ki;
		vec3 pp = ray_pos + d * L;
		d = min(1., world(pp).x/(d * .2));
		kao = min(kao, d);// * d * sign(d));
	}

	//return vec3(kao);
	return pbr(L) * max(0.,kao) / LL;
}

/*
vec3 pointlight(vec3 lightpos, vec3 lightcolor) {
	vec3 L = lightpos - ray_pos;
	float LL = dot(L,L), Ls = sqrt(LL);
	L /= Ls;

	return dirlight(L, Ls, LL, lightcolor, roughness * roughness);
}
*/

vec3 spherelight(vec4 posr, vec3 lightcolor) {
	vec3 refl = reflect(ray, normal),
			 L = posr.xyz - ray_pos,
			 l = dot(L, refl) * refl - L;
	l = L + l * clamp(posr.w/length(l), 0., 1.);

	alpha = roughness * roughness;
	//alphaP = alpha;
	alphaP = clamp(posr.w / (2. * length(l)) + roughness * roughness, 0., 1.);

	return lightcolor * dirlight(normalize(l), dot(l,l), length(l));
}

vec4 raycast() {
	vec2 tr = trace(ray_pos, -ray);
	ray_pos -= tr.x * ray;

	normal = normalize(.00005*noise34(ray_pos*20.).xyz
			+vec3(world(ray_pos+E.yxx).x,
				world(ray_pos+E.xyx).x,
				world(ray_pos+E.xxy).x)-world(ray_pos).x);
	//if (any(isnan(normal))) normal = E.xzx; // CRAP

	/*
	normal = normalize(vec3(
		world(ray_pos+E.yxx)-world(ray_pos-E.yxx),
		world(ray_pos+E.xyx)-world(ray_pos-E.xyx),
		world(ray_pos+E.xxy)-world(ray_pos-E.xxy)));
	*/

	//return vec4(normal*.5+.5, tr);
	dist_lights = 0;

	// mindex == 0 || (anything) -- default metal
	//color = E.xxx;

	vec4 ns = noise34(floor(ray_pos*9.));
	color = vec3(10.,8.,1.) * step(.98, ns.x);
	color += vec3(2.,8.,10.) * step(.96, ns.y);
	//albedo = vec3(.56, .57, .58); // iron
	albedo = vec3(.91, .92, .92); // aluminum
	metallic = 1.;//step(.2, ns.z);
	//roughness = .55;//mix(.1,.9, mod(floor(ray_pos.x*.5)+floor(ray_pos.z*.5),2.));
	roughness = mix(.4, .5, smoothstep(.4, .6, noise34(ray_pos*20.).x));

	/*
		roughness = .55;//F[14];//mix(.15, .5, step(box3(rep3(ray_pos, vec3(2.)), vec3(.6)), 0.));
		metallic = 1.;//F[15];

		// stripes
		//color = vec3(2.) * smoothstep(.99,.999,sin(t*8.+(RX(t*.4)*ray_pos).y*2.));
		//albedo = vec3(.56,.57,.58);
		//roughness = .01 + .5 * fbm(ray_pos.xy*10.);
		//metallic = .5 + fbm(ray_pos.zx*21.)*.5;
	*/

	// mindex == 1: object
	if (tr.y == 1.) {
		albedo = vec3(.95,.93,.88); // silver
		//vec2 pp = floor((ray_pos.xz+10.) / 4.) / 5.;
		//roughness = pp.x;
		//metallic = pp.y;
		roughness = .3;
	}

	if (tr.y == 100.) {
		albedo = E.xxx;
		color = light0_col;
		roughness = 0.;
	}
	/*
	color += pointlight(.5*vec3(-20.,10.,-30.), 50.*vec3(.9,.8,.7));
	color += pointlight(vec3( 20.,10.,-30.), 50.*vec3(.9,.8,.7));
	color += pointlight(vec3( 20.,10., 30.), 50.*vec3(.9,.8,.7));
	color += pointlight(vec3(-20.,10., 30.), 50.*vec3(.9,.8,.7));
	*/

	color += spherelight(light0_pos, light0_col);
	//color += pointlight(light0_pos.xyz, light0_col);

	/*
	color += pointlight(vec3(3.,7.,-3.), 1000.*vec3(.2,.5,.9));
	color += pointlight(vec3(3.,6.,3.), 500.*vec3(.5,.9,.2));
	color += pointlight(vec3(-3.,5.,3.), 500.*vec3(.9,.5,.2));
	//color += pointlight(vec3(0.,6.,0.), vec3(100.));
	*/

	return vec4(color, tr);
}

mat3 lookat(vec3 p, vec3 a, vec3 y) {
	p = normalize(p - a),
	a = normalize(cross(y, p));
	return mat3(a, cross(p, a), p);
}

//vec3 cyl(vec3 p) { return vec3(p.x*cos(p.z*PI2), p.y, p.x*sin(p.z*PI2)); }
void main() {
	vec2 uv = X / Z(1) * 2. - 1.;
	uv.x *= Z(1).x / Z(1).y;

	light0_pos = vec4(1.*sin(t*1.1), 1., 1.*cos(t*1.1), .1);
	light0_col = vec3(40.);//*fract(t));

	//gl_FragData[0] = gl_FragData[1] = vec4(uv, sin(t), 0.); return;

	//float tl = t * .6;
	//float tt = tl;//floor(tl) + pow(fract(tl), 4.);

	//vec3 origin = vec3(F[2], F[3], F[4]);//mix(rnd(vec2(floor(tl)*3.)), rnd(vec2(floor(tl)*3.+1.)), fract(tl)).xzy;

	//vec3 origin = 30. * (vec3(F[27],F[28]+.501,F[29]) - .5);// + .1 * noise13(t*3.);
	//origin = cyl(origin) + .9*noise13(t);

	//ray_pos = vec3(F[1], F[2], F[3]);
	ray_pos = vec3(0.,1.,-2.);
	ray = - lookat(ray_pos,
			//vec3(F[4], F[5], F[6]),
			vec3(0.,1.,0.),
			vec3(0.,1.,0.))
			//vec3(F[7], 1., 0.));//vec3(0.,1.5,0.)+(-.5+.5*noise13(tt)), E.xzx);
	//origin += LAT * vec3(uv*.01, 0.);
	 * normalize(vec3(uv, -1.));//F[8]));//-D.y));
	//ray = - LAT * normalize(vec3(uv, -1.));//F[8]));//-D.y));

	//gl_FragData[0] = vec4(ray,0.); return;

	//vec3 p, n;
	gl_FragData[0] = raycast();
	gl_FragData[1] = vec4(F0, roughness);
	ray = reflect(ray, normal);
	//ray_pos -= ray * .02;
	gl_FragData[1].xyz *= raycast().xyz;

	//TODO IBL
	//gl_FragData[1].xyz = pbr(ray2) * raycast().xyz;
}
