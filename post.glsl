vec3 mergeNearFar(vec2 p) {
	p *= textureSize(S[6],0);
	vec4 near = T(6,p), far = T(7,p);
	return mix(near.xyz, far.xyz, far.w / (near.w + far.w));
}
void main() {
	vec2 uv = gl_FragCoord.xy / textureSize(S[6], 0);
	vec3 color = vec3(0.);

	uv -= .5;
	float amount = .4;
	color =
		vec3(
		mergeNearFar(.5+uv).r,
		mergeNearFar(.5+uv*(1.+.05*amount)).g,
		mergeNearFar(.5+uv*(1.+.1*amount)).b);

	// FIXME: color.w has wrong semantics
	//color.xyz = pow(color.xyz / (color.xyz + color.w), vec3(1./2.2));
	color /= color + 1.;
	color = pow(color, vec3(1./2.2));

	// grain
	//color.xyz -= .03*texture2D(S[0], uv+F[2]*vec2(600.,107.)).y;
	color-= .03*T(0, gl_FragCoord.xy*.3+F[2]*vec2(623.,1107.)).y;

	//color.xyz *= (1. - smoothstep(.5,.7,length(uv-.5)));
	uv = 1. - 2. * abs(uv);
	color*= min(1., 20.*uv.x*uv.y);

	gl_FragColor = vec4(color, 1.);
}
