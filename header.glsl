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
float t = F[16]*10.;
const vec3 E = vec3(.0,.01,1.);
const float PI = 3.141593, PI2 = PI * 2.;
#define T(s,c) texture2D(S[s], (c)/textureSize(S[s],0))
#define Q(s,c) texture2D(S[s], (c)/textureSize(S[1],0))
//vec2 uv = gl_FragCoord.xy / textureSize(S[1],0);
