void main() {
	vec4 c = vec4(0.);
	float cs = T(3,gl_FragCoord.xy).w;//, r = mod(cs,128.)/127., z = floor(cs / 128.) / 127. * 100.;
	//gl_FragColor = vec4(z/100000.); return;
	for(float y = -8.; y < 8.; ++y)
		for(float x = -8.; x < 8.; ++x) {
			vec4 s = T(3,vec2(x,y)+gl_FragCoord.xy);
			//float sr = mod(s.w,128.)/127., sz = floor(s.w / 128.) / 127. * 100.;
			//if (abs(r-sr) < .2 && abs(z-sz) < .1)
				c += vec4(s.xyz, 1.);
		}
	gl_FragColor = vec4(c.xyz/c.w, cs);
}
