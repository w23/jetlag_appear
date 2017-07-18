void main() {
	vec4 c = T(1,gl_FragCoord.xy), r = T(2,gl_FragCoord.xy);
	gl_FragColor = vec4(
		mix(c.xyz,mix(r.xyz, T(3,gl_FragCoord.xy).xyz, r.w), .5), c.w);
}
