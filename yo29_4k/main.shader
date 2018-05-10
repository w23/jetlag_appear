//#version 130
//uniform int F[1];
float t = $(float time);
uniform sampler2D S;
vec3 E = vec3(0.,.01,1.);
vec4 noise24(vec2 v) { return texture2D(S, (v + .5)/textureSize(S,0)); }

const float R0 = 6360e3;
const float Ra = 6380e3;
const float low = 1e3, hi = 25e2, border = 5e2;
const float g = .76;
const float g2 = g * g;
const float Hr = 8e3;
const float Hm = 1.2e3;
float I = 10.;

const vec3 C = vec3(0., -R0, 0.);
const vec3 bR = vec3(58e-7, 135e-7, 331e-7);
const vec3 bMs = vec3(2e-5);
const vec3 bMe = bMs * 1.1;

vec3 sundir = vec3(1.,.001,1.);

/*
float noise31(vec3 v) {
	vec3 p = floor(v);
  vec3 f = fract(v);
	f = f*f*(3.-2.*f);

	vec2 uv = (p.xy+vec2(37.,197.)*p.z) + f.xy;
	vec2 rg = textureLod(S, (uv+.5)/textureSize(S,0), 0.).yx;
	return mix(rg.x, rg.y, f.z);
}*/

float noise31(vec3 v) {
	return mix(noise24(v.xz).x, noise24(v.yx).y, .5);// fract(v.z));
}

bool clouds = true;
vec2 densitiesRM(vec3 p) {
	float h = max(0., length(p - C) - R0);
	vec2 retRM = vec2(exp(-h/Hr), exp(-h/Hm) * 8.);

	if (clouds) {
		/*
		float r = length(p.xz);
		if (r < 100. && p.y < 500.) {
			retRM.y += 6e3 * smoothstep(.35, .55, fnoise(p*5e-2-vec3(0.,t*2.,0.))) * step(r, 8. + .3 * (p.y - 200.));
		}
		*/

		if (low < h && h < hi) {
			vec3 v = 15e-4 * (p + t * vec3(-90., 0., 80.));
			retRM.y +=
				250. *
				step(length(p), 50000.) *
				smoothstep(low, low + 1e2, h) *
				smoothstep(hi, hi - 1e3, h) *
				smoothstep(.5, .55,
					.75 * noise31(v)
					+ .125 * noise31(v*4.3 + max(t, 144.))
					+ .0625 * noise31(v*9.9)
					+ .0625 * noise31(v*17.8)-.1
				);
		}
	}

	return retRM;
}

float escape(vec3 p, vec3 d, float R) {
	vec3 v = p - C;
	float b = dot(v, d);
	float c = dot(v, v) - R*R;
	float det2 = b * b - c;
	if (det2 < 0.) return -1.;
	float det = sqrt(det2);
	float t1 = -b - det, t2 = -b + det;
	return (t1 >= 0.) ? t1 : t2;
}

vec4 random;

vec2 scatterDirectImpl(vec3 o, vec3 d, float L, float steps) {
	vec2 depthRMs = vec2(0.);
	L /= steps; d *= L;
	//o += d * noise24(o.xz*1e3 + vec2(t*1e4)).x;
	for (float i = random.w * .5; i < steps; ++i)
		depthRMs += densitiesRM(o + d * i);
	return depthRMs * L;
}

vec3 Lin(vec3 o, vec3 d, float L) {
	vec2 depthRMs = scatterDirectImpl(o, d, L, 32.);
	return exp(-(bR * depthRMs.x + bMe * depthRMs.y));
}

vec2 totalDepthRM;
vec3 LiR, LiM;

void scatterImpl(vec3 o, vec3 d, float L, float steps) {
	L /= steps; d *= L;
	for (float i = random.z; i < steps; ++i) {
		vec3 p = o + d * i;
		vec2 dRM = densitiesRM(p) * L;
		totalDepthRM += dRM;

		/*
		//vec2 depthRMsum = scatterDirectImpl(p, sundir, escape(p, sundir, Ra), 32.) + totalDepthRM;
		vec2 depthRMsum = totalDepthRM;
		float l = escape(p, sundir, R0 + low);
		if (l > 0.)
			depthRMsum += scatterDirectImpl(p, sundir, l, 4.);
		else l = 0.;
		float l2 = escape(p, sundir, R0 + hi);
		if (l2 > 0.)
			depthRMsum += scatterDirectImpl(p + sundir * l, sundir, l2, 16.);
		else l2 = 0.;
		l = escape(p, sundir, Ra);
		if (l > 0.)
			depthRMsum += scatterDirectImpl(p + sundir * l2, sundir, l, 8.);
		*/

		vec2 depthRMsum = totalDepthRM;
		float l = escape(p, sundir, R0 + hi);
		if (l > 0.)
			depthRMsum += scatterDirectImpl(p, sundir, l, 16.);
		else l = 0.;
		depthRMsum += scatterDirectImpl(p + sundir * l, sundir, escape(p, sundir, Ra), 4.);

		//vec2 depthRMsum = totalDepthRM + scatterDirectImpl(p, sundir, escape(p, sundir, Ra), 32.);

		/*
		float lesc = escape(p, sundir, Ra);
		vec2 depthRMsum = totalDepthRM + scatterDirectImpl(p, sundir, lesc, min(128., lesc / 100.));
		*/


		vec3 A = exp(-(bR * depthRMsum.x + bMe * depthRMsum.y));
		LiR += A * dRM.x;
		LiM += A * dRM.y;
	}
}

vec3 scatter(vec3 o, vec3 d, float L, vec3 Lo) {
	totalDepthRM = vec2(0.);
	LiR = LiM = vec3(0.);

	/*
	float det_dist = 1e4;
	float left = L - det_dist;
	if (left > 0.) {
		scatterImpl(o, d, det_dist, 64., totalDepthRM, LiR, LiM);
		scatterImpl(o + d * det_dist, d, left, 32., totalDepthRM, LiR, LiM);
	} else
		scatterImpl(o, d, L, 32., totalDepthRM, LiR, LiM);
	*/

	//float l = 10e3;//escape(o, d, R0 + low);//, l2 = escape(o, d, R0 + hi);
	float l = escape(o, d, R0 + low);//, l2 = escape(o, d, R0 + hi);
	if (L < l)
		scatterImpl(o, d, L, 16.);
	else {
		/*
		scatterImpl(o, d, l, 32.);
		scatterImpl(o+d*l, d, L-l, 8.);
		*/

		float l2 = escape(o, d, R0 + hi);
		scatterImpl(o, d, l, 32.);
		scatterImpl(o+d*l, d, l2-l, 48.);
		scatterImpl(o+d*l2, d, L-l2, 8.);
	}

	float mu = dot(d, sundir);
	return Lo * exp(-(bR * totalDepthRM.x + bMe * totalDepthRM.y))
		+ I * (1. + mu * mu) * (
			LiR * bR * .0596831 +
			LiM * bMs * .1193662 * (1. - g2) / ((2. + g2) * pow(1. + g2 - 2. * g * mu, 1.5)));
}


//float saturate(float f) { return clamp(f, 0., 1.); }
float vmax3(vec3 v) { return max(v.x, max(v.y, v.z)); }
float vmax2(vec2 v) { return max(v.x, v.y); }
float vmin2(vec2 v) { return min(v.x, v.y); }
//float box(vec3 p, vec3 s) { return vmax(abs(p) - s); }
//vec3 rep(vec3 p, vec3 s) { return mod(p, s) - s*.5; }
vec2 rep2(vec2 p, vec2 s) { return mod(p, s) - s*.5; }

float mindex = 0.;
vec2 mparam = vec2(0.);
float dbg = 0.;

const float guard = 4.;
vec4 rnd, rnd2;
float cellDist(vec4 cp, vec2 cell) {
	rnd = noise24(cell);
	rnd2 = noise24(cell/8.);
	float height = 3. * floor(2. + rnd.w * 4. + 20. * smoothstep(.8,.99, rnd.w) * smoothstep(.6, .9, rnd2.w));

	return min(
		min(
			max(vmax3(cp.xyz + rnd.xyz * 4.), cp.w - height),
			max(vmax3(cp.xyz + rnd.zxw * 7.), cp.w - height - 3. * floor(rnd.w * 4.))),
		max(vmax3(cp.yzx + 10. * rnd.xyz), cp.w - height + 3. * floor(rnd.y * 2.)));
}

float world(vec3 p) {
	vec2 road_normal = normalize(vec2(-1.,2.));
	float highway = 12. - abs(dot(road_normal,p.xz)),
		highway_length = dot(vec2(road_normal.y, -road_normal.x), p.xz);

	vec2 cell = floor(p.xz / 30.);
	p.xz = rep2(p.xz, vec2(30.));
	vec3 cell_border = vec3(abs(p.xz)-15., highway);
	float b = min(vmax2(cell_border.xy), -cell_border.z);
	float cd = cellDist(vec4(cell_border + guard, p.y), cell);
	mparam =
		mix(
			vec2(
				vmax2(cell_border.xy) + guard,
				vmin2(cell_border.xy)),
			vec2(
				cell_border.z,
				highway_length)
		, step(0., cell_border.z));

	float d = p.y;
	mindex = 1.;
	if (cd < d) {
		d = cd;
		mindex = 0.;
		mparam = cell_border.xy;
	}
	return min(d, -b + guard);
}

float L;
vec3 O, D, P, N;
void march(float maxL) {
	for (int i = 0; i < 800; ++i) {
		P = O + D * L;
		float dd = world(P);
		L += dd;
		if (dd < .0003 * L  || L > maxL) return;
	}
	L = maxL;
}

float m_kd = .3;
float m_shine;
float dirlight(vec3 dir) {
	return mix(
		max(0., dot(N, dir)) / 3.,
		pow(max(0., dot(N, normalize(dir - D))), m_shine) * (m_shine + 8.) / 24.,
		m_kd
	);
}
float poslight(vec3 pos) {
	pos -= P;
	return dirlight(normalize(pos)) / dot(pos, pos);
}

void main() {
	random = noise24(gl_FragCoord.xy + t * 1e4);
	vec2 uv = (gl_FragCoord.xy + random.xy)/$(vec2 resolution)* 2. - 1.; uv.x *= $(vec2 resolution).x / $(vec2 resolution).y;

	//if (gl_FragCoord.y < 10.) { gl_FragColor = vec4(vec3(step(gl_FragCoord.x / $(vec2 resolution).x, t / 232.)), 1.); return; }

	vec3 at = vec3(0.);
	O = vec3(mod(t*2., 1000.) - 500., 200. + 150. * sin(t/60.), mod(t*10., 1000.) - 500.);

	if (t < 32.) {
		at = vec3(1e3, 4600., -1e4);
	} else if (t < 64.) {
		O.y = 100.;
		float k = (t - 64.) / 64.;
		O = vec3(k * 100., 250., -36.);
		at = O+vec3(-30., 0., -30.);
		at.y = 10.;
		sundir.y += .01 * k * k * k;
	} else if (t < 144.) {
		sundir.y = .01;
	} else {
		float k = (t - 144.) / 64.;
		O = vec3(mod(k*2., 1000.) - 900., 100. - 40. * k, mod(k*60., 1000.) - 500.);
		at.y = 40. * k;
		//sundir.y = .01 + .5* k;// * k * k * k;
		sundir.y = .01 + 2.* k;// * k * k * k;
	}

	I = 10. + 200. * max(0., (t - 208.) / 16.);

	sundir = normalize(sundir);
	D = normalize(O - at);

	//O = $(vec3 cam_pos) * 3.; D = -normalize($(vec3 cam_dir));

	vec3 x = normalize(cross(E.xzx, D));
	D = mat3(x, normalize(cross(D, x)), D) * normalize(vec3(uv, -2)); // + noise24(uv+t*1e3).xy / $(vec2 resolution), -2.));
	//D = mat3(x, normalize(cross(D, x)), D) * normalize(vec3(uv + noise24(uv+t*1e3).xy / $(vec2 resolution), -2.));

	vec3 color = vec3(0.);

	const float max_distance = 1e4;
	march(max_distance);
	if (L < max_distance) {
		N = normalize(vec3(
			world(P+E.yxx), world(P+E.xyx), world(P+E.xxy)) - world(P));

		vec2 f = vec2(vmax2(-mparam), P.y),
			fxy = mod(f, vec2(3.)),
			fxyC = floor(f / 3.);
		float window = step(1.3, fxy.y) * step(.8, fxy.x);

		vec3 albedo = mix(vec3(.1) + .02 * rnd.yzw, vec3(.2), window) * (.1 + .9 * smoothstep(0., 24., P.y));
		m_kd = mix(.3, .9, window);
		m_shine = mix(18., 1e4, window);

		//N += .01 * (noise31(P*4.) - .5);

		if (mindex > 0.) {
			window = 0.;
			m_kd = .2;
			m_shine = 80.;
			albedo = mix(
				vec3(.3) * (.1 + .3 * smoothstep(-2., 1., mparam.x)),
				vec3(.01 + .9
					* step(3.9, mod(mparam.x, 4.)
					* step(.5, fract(mparam.y/4.)))
				),
				step(1., mparam.x));
			
			/*
			color =
				100. * vec3(.9, .1, .05)
					* step(2.9, mod(mparam.x+1., 4.))
					* step(.8, fract((mparam.y - t * 20.)/16.))
				* step(1., mparam.x);
			*/
		}

		color += Lin(P, sundir, escape(P, sundir, Ra)) * I * dirlight(sundir);

		color += 250. * mix(
				vec3(.9, .5, .2),
				vec3(.2, .5, .9),
				rnd.z * .5)
			* rnd.w
			* poslight(vec3(floor((P.xz + 15.) / 30.) * 30. + 15. * (rnd.xy - .5), 3.).xzy)
			* smoothstep(144., 140., t - rnd.w * 4. - rnd2.w * 8.)
			;

		vec3 opposite = vec3(-sundir.x, sundir.y, -sundir.z);
		clouds = false;
		//m_kd = .5;
		color += scatter(P, opposite, escape(P, opposite, Ra), vec3(0.)) * 6. * dirlight(opposite);// * m_kd * max(0., dot(N, opposite));// / 3.;
		clouds = true;
		color = scatter(O, D, L, color * albedo);

		//vec4 wrnd = noise24(floor(wxy) + t);
		vec4 wrnd = noise24(floor(fxyC + P.xz/3.));
		//vec4 wrnd = noise24(floor(vec2(vmax2(P.xz/2.), P.y)));
		color += window
			* (vec3(.2 + .2 * wrnd.xyz)
				//	+ .3 * noise24(fxy*3.).yzw
				)
			* smoothstep(.85, 1., wrnd.w + .1 * smoothstep(.34, .8, .2 * rnd.w + .8 * rnd2.w) 
					- max(0., t - 136.)/64.)
			;
	} else
		color = scatter(O, D, escape(O, D, Ra), vec3(0.));

	//color = vec3(fract(steps/100.));

	//gl_FragColor = color.x < 0.0001 ? vec4(1.,0.,0.,1.) : vec4(pow(color, vec3(1./2.2)),.5);
	gl_FragColor = smoothstep(0., 32., t) * vec4(pow(color, vec3(1./2.2)),.3);
}
