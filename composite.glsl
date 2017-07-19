void main() {
#if 0
	gl_FragColor = T(1,gl_FragCoord.xy);
#else
	vec4 c = T(1,gl_FragCoord.xy), r = T(2,gl_FragCoord.xy);
	gl_FragColor = vec4(
		mix(c.xyz,mix(r.xyz, T(4,gl_FragCoord.xy).xyz, r.w), .5), c.w);
#endif
}
