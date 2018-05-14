uniform sampler2D S;
uniform int F;
float t = float(F) / 352800.;
vec4 noise24(vec2 v) { return texture2D(S, (v + .5)/1024.); }

/*const*/ float R0 = 6360e3;
/*const*/ float Ra = 6380e3;
/*const*/ float low = 1e3, hi = 25e2, border = 5e2;
///*const*/ float Hr = 8e3, Hm = 12e2;
float I = 10.;

vec3 E = vec3(0.,.01,1.);
/*const*/ vec3 C = vec3(0., -R0, 0.);
/*const*/ vec3 bR = vec3(58e-7, 135e-7, 331e-7);
/*const*/ vec3 bMs = vec3(2e-5);
/*const*/ vec3 bMe = bMs * 1.1;

vec3 sundir = vec3(1.,.01,1.);

float noise31(vec3 v) {
	return (noise24(v.xz).x + noise24(v.yx).y) * .5;
}

//int clouds = 1;
vec2 densitiesRM(vec3 p) {
	float h = max(0., length(p - C) - R0);
	vec2 retRM = vec2(exp(-h/8e3), exp(-h/12e2) * 8.);
	//vec2 retRM = vec2(exp(-h/Hr), exp(-h/Hm) * 8.);

	if (/*clouds > 0 && */low < h && h < hi) {
		vec3 v = 15e-4 * (p + t * vec3(-90., 0., 80.));
		retRM.y +=
			250. *
			//step(length(p), 50000.) *
			//smoothstep(48., 0., t - 160.) *
			//smoothstep(28., 23., v.z) *
			step(v.z, 38.) *
			smoothstep(low, low + 1e2, h) *
			smoothstep(hi, hi - 1e3, h) *
			smoothstep(.5, .55,
				.75 * noise31(v)
				+ .125 * noise31(v*4. + t)//max(144.,t))
				+ .0625 * noise31(v*9.)
				+ .0625 * noise31(v*17.)-.1
			);
	}

	return retRM;
}

float escape(vec3 p, vec3 d, float R) {
	vec3 v = p - C;
	float b = dot(v, d);
	float c = dot(v, v) - R*R;
	float det = b * b - c;
	if (det < 0.) return -1.;
	det = sqrt(det);
	float t1 = -b - det, t2 = -b + det;
	return (t1 >= 0.) ? t1 : t2;
}

vec4 pixel_random;
vec4 kuruma_random;

vec2 scatterDirectImpl(vec3 o, vec3 d, float L, float steps) {
	vec2 depthRMs = vec2(0.);
	L /= steps; d *= L;
	for (float i = pixel_random.w * .5; i < steps; ++i)
		depthRMs += densitiesRM(o + d * i);
	return depthRMs * L;
}

vec3 Lin(vec3 o, vec3 d, float L) {
	vec2 depthRMs = scatterDirectImpl(o, d, L, 32.);
	return exp(- bR * depthRMs.x - bMe * depthRMs.y);
}

vec2 totalDepthRM;
vec3 LiR, LiM;

void scatterImpl(vec3 o, vec3 d, float L, float steps) {
	L /= steps; d *= L;
	for (float i = pixel_random.z; i < steps; ++i) {
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
		float l = max(0., escape(p, sundir, R0 + hi));
		if (l > 0.)
			depthRMsum += scatterDirectImpl(p, sundir, l, 16.);
		depthRMsum += scatterDirectImpl(p + sundir * l, sundir, escape(p, sundir, Ra), 4.);

		//vec2 depthRMsum = totalDepthRM + scatterDirectImpl(p, sundir, escape(p, sundir, Ra), 32.);

		/*
		float lesc = escape(p, sundir, Ra);
		vec2 depthRMsum = totalDepthRM + scatterDirectImpl(p, sundir, lesc, min(128., lesc / 100.));
		*/


		vec3 A = exp(-bR * depthRMsum.x - bMe * depthRMsum.y);
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
		//scatterImpl(o, d, l, 32.);
		//scatterImpl(o+d*l, d, L-l, 8.);

		float l2 = escape(o, d, R0 + hi);
		scatterImpl(o, d, l, 16.);
		scatterImpl(o+d*l, d, l2-l, 40.);
		scatterImpl(o+d*l2, d, L-l2, 8.);
	}

	float mu = dot(d, sundir);

	return Lo * exp(-bR * totalDepthRM.x - bMe * totalDepthRM.y)
		+ I * (1. + mu * mu) * (
			LiR * bR * .0597 +
			LiM * bMs * .0196 / pow(1.58 - 1.52 * mu, 1.5));
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
/*const*/ float guard = 4.;
vec4 rnd, rnd2;

/*const*/ vec2 road_normal = normalize(vec2(-1.,2.)), road_tangent = vec2(road_normal.y, - road_normal.x);//mat2(0., 1., -1., 0.) * road_normal;

float world(vec3 p) {
	vec2 pxz = p.xz;
	float
		highway = 12. - abs(dot(road_normal,pxz)),
		highway_length = dot(road_tangent, pxz);

	float r = length(pxz);
	float d = p.y;
	mindex = 1.;
	if (r > 5e3)
		return d - min(1.,(r-5e3)/500.) * 400. * noise24(pxz/300.).x;

	vec2 cell = floor(pxz / 30.);
	pxz = rep2(pxz, vec2(30.));
	rnd = noise24(cell);
	rnd2 = noise24(cell/8.);

	vec3 cell_border = vec3(abs(pxz)-15., highway);
	vec3 cp = vec3(cell_border + guard);
	//float height = 3. * floor(2. + rnd.w * 4. + 20. * smoothstep(.8,.99, rnd.w) * smoothstep(.6, .9, rnd2.w));
	float height = 3. * floor(2. + rnd.w * 4. + 20. * smoothstep(.8,.99, rnd.w) * smoothstep(.6, .9, rnd2.w));
	float cd = min(
		min(
			max(vmax3(cp.xyz + rnd.xyz * 4.), p.y - height),
			max(vmax3(cp.xyz + rnd.zxw * 7.), p.y - height - 3. * floor(rnd.w * 4.))),
		max(vmax3(cp.yzx + 10. * rnd.xyz), p.y - height + 3. * floor(rnd.y * 2.)));

	mparam =
		mix(
			vec2(
				vmax2(cell_border.xy) + guard,
				vmin2(cell_border.xy)),
			vec2(
				cell_border.z,
				highway_length)
		, step(0., cell_border.z));

	if (cd < d) {
		d = cd;
		mindex = 0.;
		mparam = cell_border.xy;
	}
	return max(p.y-90., min(d, guard - min(vmax2(cell_border.xy), -cell_border.z)));
}

vec3 O, D, P, N;

float L = 0.;
float m_kd, m_shine;
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

vec3 kuruma(float t, float offset) {
	return vec3(
		(fract(t * (.009 - .002 * kuruma_random.x) + kuruma_random.y) * 1000. - 600. + offset) * road_tangent
			+ sign(t) * 3. * (floor(kuruma_random.z * 3.) + 1.) * road_normal
		, 1.).xzy;
}

void main() {
	vec2 F = vec2(1280.,720.);
	pixel_random = noise24(gl_FragCoord.xy + t * 1e4);

	//if (gl_FragCoord.y < 10.) { gl_FragColor = vec4(vec3(step(gl_FragCoord.x / F.x, t / 232.)), 1.); return; }

	vec3 at = vec3(0.);
	O = vec3(
		-300.,
		200.,
		mod(t, 64.) * 4.
		);


	if (t < 32.) {
		at = vec3(0., 340., 200.);
	} else if (t < 64.) {
		float k = (t - 64.) / 64.;
		O = vec3(400., 250., k * 100.);
		at = O + vec3(-30., 0., -30.);
		at.y = 10.;
		sundir.y += .01 * k * k * k;
	} else if (t < 128.) {
	} else if (t < 144.) {
		O.z -= 300.;
		O.x += 100.;
		at = vec3(O.x+9., 90., O.z+20.);
		O.y = 100.;
	//	sundir.y = .01;
	} else {
		float k = (t - 144.) / 64.;
		//-18, 70, 1
		at = vec3(500., smoothstep(207., 232., t) * 800., 200.);
		O = vec3(
			-400 + k * 100.,
			230. - k * 10.,
			-130. - k * 140.
			);
		sundir.y = .01 + k * 1.5;// * k * k * k;
	}

	//I = 10. + 200. * max(0., (t - 208.) / 16.);

	sundir = normalize(sundir);
	D = normalize(O - at);

	//O = vec3(-1.000,4.000,8.000) * 3.; D = -normalize(vec3(0.700,-0.170,0.670));

	vec3 x = normalize(cross(E.xzx, D));
	D = mat3(x, normalize(cross(D, x)), D) * normalize(vec3((gl_FragCoord.xy + pixel_random.xy)/F.xy * 2. - 1., -2.) * vec3(F.x / F.y, 1., 1.));

	vec3 color = E.xxx;

	/*const*/ float maxL = 1e4;
	for (float i = 0; i < 800.; ++i) {
		P = O + D * L;
		float dd = world(P);
		L += dd;
		if (dd < L*.0001 || L > maxL) break;
	}
	if (L < maxL) {
		N = normalize(vec3(world(P+E.yxx), world(P+E.xyx), world(P+E.xxy)) - world(P));

		vec2 f = vec2(vmax2(-mparam), P.y),
			fxy = mod(f, vec2(3.)),
			fxyC = floor(f / 3.);
		float window = step(1.3, fxy.y) * step(.8, fxy.x);

		vec3 albedo = (vec3(.1) + .02 * rnd.yzw * window) * (.1 + .9 * smoothstep(0., 24., P.y));
		m_kd = .3 + .5 * window;
		m_shine = 18. + 1e4 * window;

		//N += .01 * (noise31(P*4.) - .5);

		if (mindex > 0.) {
			//window = 0.;
			m_kd = .2;
			m_shine = 80.;
			albedo = mix(
				vec3(.1),// * (.1 + .3 * smoothstep(-2., 1., mparam.x)),
				//vec3(.1 + .3 * smoothstep(-2., 1., mparam.x)),
				vec3(.01 + .9
					* step(3.9, mod(mparam.x, 4.))
					* step(.5, fract(mparam.y/4.))
				),
				step(1., mparam.x));
		}

		color += Lin(P, sundir, escape(P, sundir, Ra)) * I * dirlight(sundir);

		vec2 streetlight_cell = floor((P.xz + 15.) / 30.);
		//color += noise24(streetlight_cell).xzy;
		color += 250. * mix(
				vec3(.9, .5, .2),
				vec3(.2, .5, .9),
				rnd.z * .5)
			* rnd.w
			* poslight
				(vec3(30. * (streetlight_cell + (noise24(streetlight_cell).xy - .5)), 3.).xzy)
			/* 7 bytes */ //* smoothstep(144., 140., t - rnd.w * 4. - rnd2.w * 8.)
			;

		// 103 bytes
		for (float i = 0; i < 20.; ++i) {
			/*
			vec4 r = noise24(vec2(i));
			vec2 side = 3. * (floor(r.w * 3.) + 1.) * road_normal;
			vec2 pos1 = (fract(-t * (.009 - .002 * r.x) + r.y) * 1000. - 600.) * road_tangent + side;
			vec2 pos2 = (fract(t * (.009 - .002 * r.x) + r.y) * 1000. - 600.) * road_tangent - side;
			color += (poslight(vec3(pos1, 1.).xzy) + poslight(vec3(pos2 - road_tangent * 3., 1.).xzy)) * vec3(30., 0., 0.);// 30. * vec3(.9, .1, .0);
			color += (poslight(vec3(pos2, 1.).xzy) + poslight(vec3(pos1 - road_tangent * 3., 1.).xzy)) * vec3(30.);//30. * vec3(.8, .8, .9);
			*/
			kuruma_random = noise24(vec2(i));
			color += (poslight(kuruma(-t, 0.)) + poslight(kuruma(t, 3.))) * vec3(30.)
				+ (poslight(kuruma(t, 0.)) + poslight(kuruma(-t, 3.))) * vec3(30.,0.,0.);
		}

		/*
		vec3 opposite = vec3(-sundir.x, sundir.y, -sundir.z);
		clouds = 0;
		color += scatter(P, opposite, escape(P, opposite, Ra), vec3(0.)) * 6. * dirlight(opposite);
		clouds = 1;
		*/
		color = scatter(O, D, L, color * albedo);

		vec4 wrnd = noise24(floor(fxyC + P.xz/3.));
		color += window
			* smoothstep(800., 600., length(P.xz))
			/* -8 bytes */ * (vec3(.2, .15, .1) + .1 * wrnd.xyz)
				//	+ .3 * noise24(fxy*3.).yzw
			* step(.4, wrnd.w*wrnd.z - max(0., t - 136.)/64.)
			//* smoothstep(.85, 1., wrnd.w + .1 * smoothstep(.34, .8, .2 * rnd.w + .8 * rnd2.w) - max(0., t - 136.)/64.)
			;
	} else
		color = scatter(O, D, escape(O, D, Ra), vec3(0.));

	//gl_FragColor = color.x < 0.0001 ? vec4(1.,0.,0.,1.) : vec4(pow(color, vec3(1./2.2)),.5);
	//gl_FragColor = vec4(pow(smoothstep(0., 32., t) * color, vec3(1./2.2)),.3);
	gl_FragColor = vec4(sqrt(smoothstep(0., 32., t) * color), .3);
}
