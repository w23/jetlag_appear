void main() {
	vec4 c = vec4(0.);
	float
		r = T(2,gl_FragCoord.xy).w,
		z = T(1,gl_FragCoord.xy).w;
	for(float y = -8.; y < 8.; ++y)
		for(float x = -8.; x < 8.; ++x) {
			vec2 v = 3.*vec2(x,y)+gl_FragCoord.xy+.5;
			vec4 s = T(2,v);
			float sr = s.w, sz = T(1,v).w;
			if (abs(r-sr) < .1 && abs(z-sz) < 1.)
				c += vec4(s.xyz, 1.);
		}
	gl_FragColor = vec4(c.xyz/max(1.,c.w), r);
}
