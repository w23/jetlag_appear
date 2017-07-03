uniform sampler2D S[8];
uniform vec3 F[32];
vec3 V = F[0];

void main() {
	//if (gl_FragCoord.y < 10.) { gl_FragColor = vec4(step(gl_FragCoord.x / V.x, TPCT)); return; }
	gl_FragColor = texture2D(S[2], gl_FragCoord.xy / V.xy);
}
