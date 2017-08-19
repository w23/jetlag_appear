void main() {
	vec2 uv = X / Z(4);
	vec4 s, c = E.xxxx;
	float
		r = P(3,uv).w,
		z = P(2,uv).w,
		x, y, sr, sz;
	/*
	for(y = -8.; y < 8.; ++y)
		for(x = -8.; x < 8.; ++x) {
			vec2 v = uv + (2.*vec2(x,y)+.0) / Z(3);
			*/
	float rad = 0.;
	vec2 angle = vec2(0.,1.1);
	mat2 rot = mat2(cos(2.4),sin(2.4),-sin(2.4),cos(2.4));
	for (int i = 0; i < 200; i++) {
		vec2 v = uv + (rad * angle + .0)/Z(4);
		rad += 1. / (rad + 4.);
		angle *= rot;

			s = P(3,v);
			sr = s.w;
			sz = P(2,v).w;
			if (abs(r-sr) < .04 && abs(z-sz) < .1)
				c += vec4(s.xyz, 1.);
			//if (any(isnan(P(2,v)))) {gl_FragColor = E.xzxx; return;}
		}
	gl_FragColor = vec4(c.xyz/max(1.,c.w), r);
}

