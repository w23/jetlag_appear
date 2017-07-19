void main() {
	vec4 c = vec4(0.);
	for(float y = -8.; y < 8.; ++y)
		for(float x = -8.; x < 8.; ++x)
			c += vec4(T(2,vec2(x,y)+gl_FragCoord.xy).xyz, 1.);
	gl_FragColor = vec4(c.xyz/c.w, T(2, gl_FragCoord.xy).w);
}
