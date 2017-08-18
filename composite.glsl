//float linstep(float a, float b, float v) { return clamp((v - a) / (b - a), 0., 1.); }
void main() {
	vec2 uv = X / Z(4);
	vec4 c = T(1,X), r = T(2,X), r2 = P(3,uv);
	//gl_FragColor = T(1,X); return;
	//gl_FragColor = vec4(c.xyz + T(4,X).xyz, c.w); return;
	gl_FragColor = vec4(mix(c.xyz,mix(r.xyz, r2.xyz, smoothstep(.2,.5,r.w)), .2), c.w);
}
