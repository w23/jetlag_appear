void main() {
	float rad = 0.;
	vec2 pixel = 2.*vec2(V.y/V.x, 1.), angle = vec2(0.,1.1);
	mat2 rot = mat2(cos(2.4),sin(2.4),-sin(2.4),cos(2.4));
	vec4 near, far;
	near = far = vec4(.0,.0,.0,.0001);
	float fdist = 1. + F[13] * 50.;
	float z = T(5,gl_FragCoord.xy).w;
	for (int i = 0; i < 64; i++) {
		vec4 pix = T(5, gl_FragCoord.xy + pixel * rad * angle);

		//float CoC = abs(100. * .5 * (D.z - sample.w) / sample.w / (D.z - .5));
		//gl_FragColor = vec4(fract(CoC), fract(CoC/10.), fract(CoC/100.), 0.);//min(1., CoC));
		//gl_FragColor = vec4(CoC/10.);
		//return;

		if (abs(50. * (fdist - pix.w) / pix.w / (fdist - .5)) > rad) {
		 	if (pix.w > fdist)
				far += vec4(pix.xyz, 1.);
			else
				near += vec4(pix.xyz, 1.);
		}

		rad += 1. / (rad + 1.);
		angle *= rot;
	}
	gl_FragData[0] = near/near.w;
	gl_FragData[1] = far/far.w;
}
