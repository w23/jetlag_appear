void main() {
#if 0
	gl_FragColor = T(1,gl_FragCoord.xy);
#else
	vec4 c = T(1,gl_FragCoord.xy),
			 r = mod(T(2,gl_FragCoord.xy),32.)/31.;
			 //r = T(2,gl_FragCoord.xy);
	gl_FragColor = vec4(c.xyz + T(4,gl_FragCoord.xy).xyz, c.w); return;
	gl_FragColor = vec4(
		mix(c.xyz,
			mix(r.xyz,
				T(4,gl_FragCoord.xy).xyz,
				r.w), .5
		//	*step(r.w*r.w, .4)
			), c.w);
#endif
}
