void main() {
	vec2 uv = X / Z(5);
	float rad = 0.;
	vec2 angle = vec2(0.,1.1);
	mat2 rot = mat2(cos(2.4),sin(2.4),-sin(2.4),cos(2.4));
	vec4 near = E.xxxx, far = E.xxxx;
	float z = P(4,uv).w;
	for (int i = 0; i < 100; i++) {
		vec4 pix = P(4, uv + (rad * angle + .0)/Z(5));

		float A = .04;
		float D = 3.;//F[9];
		float F = .03 / tan(.7);
		float P_ = pix.w;
		float coc = abs(A * F / (P_ - F) * (P_ / D - 1.)) * Z(1).x / .035;

		if (coc >= rad) {
		 	if (pix.w > D)
				far += vec4(pix.xyz, 1.);
			else
				near += vec4(pix.xyz, 1.);
		}

		rad += 1. / (rad + 10.);
		angle *= rot;
	}
	gl_FragData[0] = vec4(near.xyz, near.w);///max(.00001,near.w);
	gl_FragData[1] = vec4(far.xyz, far.w);///max(.00001,far.w);
}
