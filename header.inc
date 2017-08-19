_header_glsl:
	db `#version 130\n`
	db `#define Z(s)textureSize(S[s],0)\n`
	db `#define P(s,c)texture2D(S[s],c)\n`
	db `#define T(s,c)P(s,(c)/Z(s))\n`
	db `#define X gl_FragCoord.xy\n`
	db `#define k(x,y)texture2D(S[1],(vec2(x,y)+.5)/Z(1))\n`
	db `uniform sampler2D S[8];`
	;db `uniform float F[32];`
	db `uniform int F;`
	db `float t=float(F)/352800.;`
	db `const vec3 E=vec3(0.,.001,1.);`
	db `const float PI=3.14159;`;,PI2=PI*2.;`
	db 0
