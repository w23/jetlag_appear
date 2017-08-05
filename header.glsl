#version 130
uniform sampler2D S[12];
uniform float F[32];
float t = F[0]; // TODO F[0]
const vec3 E = vec3(.0,.01,1.);
const float PI = 3.141593, PI2 = PI * 2.;
const float rt = .1, zt = .3;
#define Z(s) textureSize(S[s], 0)
#define T(s,c) texture2D(S[s], (c)/Z(s))
#define X gl_FragCoord.xy
//vec2 uv = gl_FragCoord.xy / textureSize(S[1],0);
