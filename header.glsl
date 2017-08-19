#version 130
#define Z(s) textureSize(S[s], 0)
#define T(s,c) texture2D(S[s], (c)/Z(s))
#define P(s,c) texture2D(S[s], (c))
#define X gl_FragCoord.xy
uniform sampler2D S[8];
//uniform float F[32];
uniform int F;
float t = float(F)/352800.;
const vec3 E = vec3(.0,.001,1.);
const float PI = 3.141593;//, PI2 = PI * 2.;
//float s(float v, float m, float M) { return min(max(v, m), M); }
