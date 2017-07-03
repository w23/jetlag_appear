uniform sampler2D S[8];
uniform vec3 F[32];
vec3 V = F[0];

void main() {
	vec2 uv = gl_FragCoord.xy / V.xy, pixel = .002 * vec2(V.y/V.x, 1.), angle = vec2(0.,1.1);
	vec4 color = vec4(0.);

	float rad = 0.;
	mat2 rot = mat2(cos(2.4),sin(2.4),-sin(2.4),cos(2.4));
	for (int i = 0; i < 256; i++) {
		vec4 sample = texture2D(S[1], uv + pixel * rad * angle);

		//float CoC = abs(100. * .5 * (D.z - sample.w) / sample.w / (D.z - .5));
		//gl_FragColor = vec4(fract(CoC), fract(CoC/10.), fract(CoC/100.), 0.);//min(1., CoC));
		//gl_FragColor = vec4(CoC/10.);
		//return;

		if (abs(50. * (F[3].z - sample.w) / sample.w / (F[3].z - .5)) > rad)
			color += vec4(sample.xyz, 1.);

		rad += 1. / (rad + 1.);
		angle *= rot;
	}

	gl_FragColor = vec4(pow(color.xyz / (color.xyz + color.w), vec3(1./2.2)), 1.);
}
