uniform sampler2D S[8];
uniform float F[32];
vec3 V = vec3(F[0], F[1], F[2]),
		 C = vec3(F[3], F[4], F[5]),
		 A = vec3(F[6], F[7], F[8]),
		 D = vec3(F[9], F[10], F[11]);
float t = F[11];

void main() {
	gl_FragColor = texture2D(S[2], gl_FragCoord.xy / V.xy);
}
