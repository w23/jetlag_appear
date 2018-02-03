vec4 rnd(vec2 n) { return T(0,n-.5); }//texture2DLod(S[0], (n-.5)/textureSize(S[0],0), 0); }
float noise(float v) { return rnd(vec2(v)).w; }
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

//float box2(vec2 p, vec2 s) { p = abs(p) - s; return max(p.x, p.y); }
//float box3(vec3 p, vec3 s) { p = abs(p) - s; return max(p.x, max(p.y, p.z)); }
//mat3 RX(float a){ float s=sin(a),c=cos(a); return mat3(1.,0.,0.,0.,c,-s,0.,s,c); }
mat3 RY(float a){	float s=sin(a),c=cos(a); return mat3(c,0.,s,0.,1.,0,-s,0.,c); }
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

float techfld(vec3 p) {
	p=RZ(t*.1)*p;
	p.y+=t*.1;
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
	//p.y -= 3.;
	float d = -box(p, vec3(8., 8., 6.));
	d = max(d, box(rep3(p,vec3(1.)), vec3(.4)));
	return vec2(d, 0.);
}
float t2 = mod(t,2.),t4 = mod(t,4.), t8 = mod(t,8.);
//float ttor = smoothstep(.0,.1,t2)*smoothstep(1.,.9,t2);//step(2.,t4)*step(t4,3.);
//float or = step(40.,t)*4.*ttor-2., ok = 1.4;
vec2 object(vec3 p) {
	//p.y -= 1.;
	float d = length(p) - 2. - pow(sin(p.y*.7+t/4.),2.);
	//if (d < ok)
	//	d = max(d, min(d+.4, techfld(RZ(.7)*p*2.)*.5));
		d = max(d, min(d+.4, techfld(p*2.)*.5));
	return vec2(d, 1.);
}

vec4 light0_pos, light1_pos;
vec3 light0_col, light1_col;
//int dist_lights = 1;
vec2 world(vec3 p) {
	//mindex = 0;
	vec2 w = room(p), d;
	d = object(p);
	w = w.x < d.x ? w : d;
	/*
	if (dist_lights != 0) {
		d = vec2(length(p-light0_pos.xyz) - light0_pos.w, 10.);
		w = w.x < d.x ? w : d;
		d = vec2(length(p-light1_pos.xyz) - light1_pos.w, 20.);
		w = w.x < d.x ? w : d;
	}
	*/
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
float alpha, alphaP;

float GeometrySchlickGGX(float NV) {
	roughness += 1.; roughness *= roughness / 8.;
	return NV / (NV * (1. - roughness) + roughness);
}
vec3 pbr(vec3 lightdir) {
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

vec3 spherelight(vec4 posr, vec3 lightcolor) {
//lightcolor=vec3(1.)*40.;
	vec3 refl = reflect(ray, normal),
			 L = posr.xyz - ray_pos,
			 l = dot(L, refl) * refl - L;
	l = L + l * clamp(posr.w/length(l), 0., 1.);

	alpha = roughness * roughness;
	alphaP = clamp(posr.w / (2. * length(l)) + roughness * roughness, 0., 1.);

	return lightcolor * dirlight(normalize(l), dot(l,l), length(l));
}

vec4 raycast() {
	vec2 tr = trace(ray_pos, -ray);
	ray_pos -= tr.x * ray;

	normal = normalize(
		.00005*noise34(ray_pos*20.).xyz+
			vec3(world(ray_pos+E.yxx).x,
				world(ray_pos+E.xyx).x,
				world(ray_pos+E.xxy).x)-world(ray_pos).x);
	//if (any(isnan(normal))) normal = E.xzx; // CRAP

	/*
	normal = normalize(vec3(
		world(ray_pos+E.yxx)-world(ray_pos-E.yxx),
		world(ray_pos+E.xyx)-world(ray_pos-E.xyx),
		world(ray_pos+E.xxy)-world(ray_pos-E.xxy)));
	*/

	//dist_lights = 0;

	// mindex == 0 || (anything) -- default metal
	//color = E.xxx;

	albedo = vec3(.9);
	//albedo = vec3(.56, .57, .58); // iron
	//albedo = vec3(.91, .92, .92); // aluminum
	vec4 ns = pow(noise34(floor(ray_pos)), vec4(3.));
	metallic = 1. - ns.w;

	ns = pow(noise34(floor(ray_pos)+floor(t*4.)), vec4(3.));
	color = vec3(
		step(.7,ns.w)*10.
		*step(t, 88.)
		*max(
			step(t, 32.),
			step(56., t))
		*max(
			(1.-smoothstep(1.,4.,mod(t,8.))),
			(1.-smoothstep(1.,4.,mod(t,4.)))*step(16.,t))

		);
	roughness = 0.;//smoothstep(.1, .9, pow(noise34(ray_pos*20.).x, 2.));

	// mindex == 1: object
	if (tr.y == 1.) {
		color = E.xxx;
		//color = vec3(4.) * smoothstep(.99,.999,sin(t+(/*RX(t*.4)*/ray_pos).y*4.));
		//albedo = vec3(.95,.93,.88); // silver
		metallic = 0.;
		roughness = .9;
	}
/*
	if (tr.y == 10.) {
		albedo = E.xxx;
		color = light0_col;
		roughness = 0.;
	}
	if (tr.y == 20.) {
		albedo = E.xxx;
		color = light1_col;
		roughness = 0.;
	}
*/
	color += spherelight(light0_pos, light0_col);
	color += spherelight(light1_pos, light1_col);

	return vec4(color, tr);
}

mat3 lookat(vec3 p, vec3 a, vec3 y) {
	p = normalize(p - a),
	a = normalize(cross(y, p));
	return mat3(a, cross(p, a), p);
}

void main() {
	vec2 uv = X / Z(1) * 2. - 1.;
	uv.x *= Z(1).x / Z(1).y;

	light0_pos = vec4(2., 7., 3., .1);
	light1_pos = vec4(-3., -4., 4., .2);

	float lt = smoothstep(24., 32., t)*smoothstep(88.,80.,t)*40.;
	light0_col = vec3(.2,.8,.9) * lt;
	light1_col = vec3(.7,.9,.6) * lt;

	ray_pos = RZ(t*.09)*RY(t*.1)*(vec3(0.,0.,4.+noise(t*.21))+.9*rnd(vec2(t*.25)).wyx);
	vec3 at = .9*rnd(vec2(t*.05)).wyx;
	ray = - lookat(
			ray_pos,
			at,
			vec3(0.,1.,0.))
	 * normalize(vec3(uv, -1.));//F[8]));//-D.y));

	gl_FragData[0] = raycast();
	gl_FragData[1] = vec4(F0, roughness);
	ray = reflect(ray, normal);
	//ray_pos -= ray * .02;
	gl_FragData[1].xyz *= raycast().xyz;
}
