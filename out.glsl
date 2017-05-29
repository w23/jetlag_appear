uniform sampler2D B;
uniform vec3 V;
uniform float TPCT;

void main() {
	if (gl_FragCoord.y < 10.) { gl_FragColor = vec4(step(gl_FragCoord.x / V.x, TPCT)); return; }
	gl_FragColor = texture2D(B, gl_FragCoord.xy / V.xy);
}
