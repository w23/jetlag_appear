float linstep(float a, float b, float v) { return clamp((v - a) / (b - a), 0., 1.); }
void main() {
	vec4 c = T(1,X), r = T(2,X);
	//gl_FragColor = T(1,X); return;
	//gl_FragColor = vec4(c.xyz + T(4,X).xyz, c.w); return;
	gl_FragColor = vec4(mix(c.xyz,
		mix(mix(r.xyz, T(3,X).xyz, linstep(.1,.2,r.w)), T(4,X).xyz, linstep(.3,.6,r.w))
	, .2), c.w);	return;

	gl_FragColor = vec4(
		mix(c.xyz,
			mix(r.xyz,
				T(4,X).xyz,
				r.w), .5
		//	*step(r.w*r.w, .4)
			), c.w);
}
