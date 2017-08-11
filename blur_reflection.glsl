void main() {
	vec2 uv = X / Z(3);
	vec4 s, c = E.xxxx;
	float
		r = P(2,uv).w,
		z = P(1,uv).w,
		x, y, sr, sz;
	/*
	for(y = -8.; y < 8.; ++y)
		for(x = -8.; x < 8.; ++x) {
			vec2 v = uv + (2.*vec2(x,y)+.5) / Z(3);
			*/
	float rad = 0.;
	vec2 angle = vec2(0.,1.1);
	mat2 rot = mat2(cos(2.4),sin(2.4),-sin(2.4),cos(2.4));
	for (int i = 0; i < 100; i++) {
		vec2 v = uv + (rad * angle + .0)/Z(3);
		rad += 1. / (rad + 4.);
		angle *= rot;
			s = P(2,v);
			sr = s.w;
			sz = P(1,v).w;
			if (abs(r-sr) < rt && abs(z-sz) < zt)
				c += vec4(s.xyz, 1.);
		}
	gl_FragColor = vec4(c.xyz/max(1.,c.w), r);
}

