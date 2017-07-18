void main() {
	vec2 uv = gl_FragCoord.xy / textureSize(S[1], 0);
	vec4 color = vec4(0.);

#if 0
	float rad = 0.;
	vec2 pixel = .002 * vec2(V.y/V.x, 1.), angle = vec2(0.,1.1);
	mat2 rot = mat2(cos(2.4),sin(2.4),-sin(2.4),cos(2.4));
	float fdist = 1. + F[13] * 50.;
	for (int i = 0; i < 256; i++) {
		vec4 sample = texture2D(S[1], uv + pixel * rad * angle);

		//float CoC = abs(100. * .5 * (D.z - sample.w) / sample.w / (D.z - .5));
		//gl_FragColor = vec4(fract(CoC), fract(CoC/10.), fract(CoC/100.), 0.);//min(1., CoC));
		//gl_FragColor = vec4(CoC/10.);
		//return;

		if (abs(50. * (fdist - sample.w) / sample.w / (fdist - .5)) > rad)
			color += vec4(sample.xyz, 1.);

		rad += 1. / (rad + 1.);
		angle *= rot;
	}
#else
	uv -= .5;
	float amount = .2;
	color = vec4(
		texture2D(S[1], .5 + uv).r,
		texture2D(S[1], .5 + uv*(1.+.05*amount)).g,
		texture2D(S[1], .5 + uv*(1.+.1*amount)).b,
		1. / (1. - 1.4*length(uv)));
#endif
	
	//uv /= textureSize(S[1],0);
	//color = vec4(length(uv)*.001);
	//color *= smoothstep(.8, .9, length(uv));

	//gl_FragColor =  vec4(pow(color.xyz / (color.xyz + color.w), vec3(1.)), 1.);
	color.xyz = pow(color.xyz / (color.xyz + color.w), vec3(1./2.2));

	// grain
	color.xyz += .03*texture2D(S[0], uv+F[2]*vec2(600.,107.)).y;
	gl_FragColor = vec4(color.xyz, 1.);
}
