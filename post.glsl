uniform sampler2D S[8];
uniform float F[32];
vec3 V = vec3(F[0], F[1], F[2]),
		 C = vec3(F[3], F[4], F[5]),
		 A = vec3(F[6], F[7], F[8]),
		 D = vec3(F[9], F[10], F[11]);
float t = F[11];

void main() {
	vec2 uv = gl_FragCoord.xy / V.xy;
	vec4 color = vec4(0.);

#if 0
	float rad = 0.;
	vec2 pixel = .002 * vec2(V.y/V.x, 1.), angle = vec2(0.,1.1);
	mat2 rot = mat2(cos(2.4),sin(2.4),-sin(2.4),cos(2.4));
	for (int i = 0; i < 256; i++) {
		vec4 sample = texture2D(S[1], uv + pixel * rad * angle);

		//float CoC = abs(100. * .5 * (D.z - sample.w) / sample.w / (D.z - .5));
		//gl_FragColor = vec4(fract(CoC), fract(CoC/10.), fract(CoC/100.), 0.);//min(1., CoC));
		//gl_FragColor = vec4(CoC/10.);
		//return;

		if (abs(50. * (F[11] - sample.w) / sample.w / (F[11] - .5)) > rad)
			color += vec4(sample.xyz, 1.);

		rad += 1. / (rad + 1.);
		angle *= rot;
	}
#endif

	color = texture2D(S[1], uv);

	gl_FragColor = vec4(pow(color.xyz / (color.xyz + color.w), vec3(1./2.2)), 1.);
}
