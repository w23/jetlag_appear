void main() {
#define TEXTURE 6
#if 0
	gl_FragColor = texture2D(S[TEXTURE], gl_FragCoord.xy / V.xy);
#else
	gl_FragColor = 10.*texture2D(S[TEXTURE], gl_FragCoord.xy / textureSize(S[TEXTURE],0));
	//gl_FragColor = 1.*texture2D(S[TEXTURE], gl_FragCoord.xy / textureSize(S[TEXTURE],0)).wwww;
#endif

}
