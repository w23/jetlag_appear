uniform vec2 VIEWPORT;
void main() {
#define K 1.
#define TEXTURE 8
	//gl_FragColor = T(TEXTURE, gl_FragCoord.xy).wwww*.1; return;
#if 0
	gl_FragColor = texture2D(S[TEXTURE], gl_FragCoord.xy / textureSize(S[TEXTURE],0));
#else
	gl_FragColor = K * texture2D(S[TEXTURE], gl_FragCoord.xy / VIEWPORT);
	//gl_FragColor = 1.*texture2D(S[TEXTURE], gl_FragCoord.xy / textureSize(S[TEXTURE],0)).wwww;
#endif
	//gl_FragColor.z += step(T(6,gl_FragCoord.xy).x,0.);

}
