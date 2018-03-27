#version 130
#define Z(s) textureSize(S[s], 0)
#define T(s,c) texture2D(S[s], (c)/Z(s))
#define P(s,c) texture2D(S[s], (c))
#define X gl_FragCoord.xy
#define k(x,y) texture2D(S[1],(vec2(x,y)+.5)/Z(1))
uniform sampler2D S[9];
uniform sampler2D OUTPUT;
//uniform float F[32];
uniform int F;
float t = float(F)/352800.;
const vec3 E = vec3(.0,.001,1.);
const float PI = 3.141593;//, PI2 = PI * 2.;
//float s(float v, float m, float M) { return min(max(v, m), M); }
uniform vec2 VIEWPORT;
void main() {
	//gl_FragColor = 1.*P(6,(X-.5)/VIEWPORT); return;
#define K 1.
//#define TEXTURE 4
	//gl_FragColor = T(TEXTURE, gl_FragCoord.xy).wwww*.1; return;
#if TEXTURE
	gl_FragColor = texture2D(S[TEXTURE], gl_FragCoord.xy / textureSize(S[TEXTURE],0));
#else
	gl_FragColor = K * texture2D(OUTPUT, gl_FragCoord.xy / VIEWPORT);
	/*
	vec2 tex_size = textureSize(OUTPUT, 0);
	float tex_aspect = tex_size.x / tex_size.y;
	float viewport_aspect = VIEWPORT.x / VIEWPORT.y;
	vec2 uv = gl_FragCoord.xy / VIEWPORT;
	float aspect_d = viewport_aspect - tex_aspect;
	if (aspect_d > 0.) {
		uv.x = uv.x * tex_aspect / viewport_aspect - aspect_d / viewport_aspect * .5;
	} else{
		uv.y = uv.y * tex_aspect / viewport_aspect - aspect_d / viewport_aspect * .5;
	}
	gl_FragColor = K * texture2D(OUTPUT, uv);
	*/
	//gl_FragColor = 1.*texture2D(S[TEXTURE], gl_FragCoord.xy / textureSize(S[TEXTURE],0)).wwww;
#endif
	//gl_FragColor.z += step(T(6,gl_FragCoord.xy).x,0.);

	//gl_FragColor = vec4(0.);
}
