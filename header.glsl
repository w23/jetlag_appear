#version 130
uniform sampler2D S[8];
uniform float F[32];
float t = F[0]; // TODO F[0]
const vec3 E = vec3(.0,.001,1.);
const float PI = 3.141593, PI2 = PI * 2.;
const float rt = .1, zt = .1;
#define Z(s) textureSize(S[s], 0)
#define T(s,c) texture2D(S[s], (c)/Z(s))
#define P(s,c) texture2D(S[s], (c))
#define X gl_FragCoord.xy
