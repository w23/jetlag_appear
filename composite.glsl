//float linstep(float a, float b, float v) { return clamp((v - a) / (b - a), 0., 1.); }
void main() {
	vec4 c = T(1,X), r = T(2,X), r2 = T(3,X);
	//gl_FragColor = T(2,X); return;
	//gl_FragColor = vec4(c.xyz + T(5,X).xyz, c.w); return;
	gl_FragColor = vec4(mix(c.xyz,mix(r.xyz, r2.xyz, smoothstep(.2,.5,r.w)), .2), c.w);
}
