void main() {
	vec2 uv = gl_FragCoord.xy / textureSize(S[1], 0);
	vec4 color = vec4(0.);


#if 1
	color = T(6,gl_FragCoord.xy) + T(7,gl_FragCoord.xy);
#else
	uv -= .5;
	float amount = .4;
	color = vec4(
		texture2D(S[5], .5 + uv).r,
		texture2D(S[5], .5 + uv*(1.+.05*amount)).g,
		texture2D(S[5], .5 + uv*(1.+.1*amount)).b,
		1. / (1. - 1.4*length(uv)));
#endif

	// FIXME: color.w has wrong semantics
	//color.xyz = pow(color.xyz / (color.xyz + color.w), vec3(1./2.2));
	color.xyz = pow(color.xyz, vec3(1./2.2));

	// grain
	//color.xyz -= .03*texture2D(S[0], uv+F[2]*vec2(600.,107.)).y;
	color.xyz -= .03*T(0, gl_FragCoord.xy*.3+F[2]*vec2(623.,1107.)).y;

	//color.xyz *= (1. - smoothstep(.5,.7,length(uv-.5)));
	uv = 1. - 2. * abs(uv - .5);
	color.xyz *= min(1., 20.*uv.x*uv.y);

	gl_FragColor = vec4(color.xyz, 1.);
}
