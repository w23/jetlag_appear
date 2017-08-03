void main() {
	vec4 s, c = E.xxxx;
	float
		r = T(3,X).w,
		z = T(1,X).w,
		x, y, sr, sz;
	for(y = -8.; y < 8.; ++y)
		for(x = -8.; x < 8.; ++x) {
			vec2 v = 3.*vec2(x,y)+X+.5;
			s = T(3,v);
			sr = s.w;
			sz = T(1,v).w;
			if (abs(r-sr) < rt && abs(z-sz) < zt)
				c += vec4(s.xyz, 1.);
		}
	gl_FragColor = vec4(c.xyz/max(1.,c.w), r);
}
/*
void main() {
	vec4 c = T(3,gl_FragCoord.xy);
	float
		r = c.w,
		z = T(1,gl_FragCoord.xy).w;
	c.w = 1.;
	for(float y = -8.; y < 8.; ++y)
		for(float x = -8.; x < 8.; ++x) {
			vec2 v = 4.*vec2(x,y)+gl_FragCoord.xy+.5;
			vec4 s = T(3,v);
			float sr = s.w, sz = T(1,v).w;
			if (abs(r-sr) < .1 && abs(z-sz) < 1.)
				c += vec4(s.xyz, 1.);
		}
	gl_FragColor = vec4(c.xyz/max(1.,c.w), r);
}*/
