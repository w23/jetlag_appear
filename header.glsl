#version 130
uniform sampler2D S[12];
uniform float F[32];
vec3 V = vec3(F[0], F[1], F[2]),
		 C = vec3(F[3], F[4], F[5]),
		 //A = vec3(F[6], F[7], F[8]),
		 A = vec3(0., 3., 0.),
		 D = vec3(F[9], F[10], F[11]);
//float t = F[11];
//float t = F[12] * 20.;
//float t = F[16]*10.;
float t = F[2];
const vec3 E = vec3(.0,.01,1.);
const float PI = 3.141593, PI2 = PI * 2.;
const float rt = .1, zt = .3;
#define Z(s) textureSize(S[s], 0)
#define T(s,c) texture2D(S[s], (c)/Z(s))
#define Q(s,c) texture2D(S[s], (c)/Z(1))
#define X gl_FragCoord.xy
//vec2 uv = gl_FragCoord.xy / textureSize(S[1],0);
