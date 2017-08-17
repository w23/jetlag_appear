vec4 mergeNearFar(vec2 p) {
	p += .5 / Z(5);
	//p *= Z(5);
	//return P(5,p);
	vec4 near = P(5,p), far = P(6,p);
	//return near;
	//return (near + far) / (near.w + far.w);
	//return near + far;
	return (near/max(.0001,near.w) * near.w + far/max(.0001,far.w) * far.w) / (near.w + far.w);
	//return vec4(step(near.w + far.w, 1.));
	//return vec4(near.w, far.w, step(near.w + far.w, 0.), 0.);
	//return vec4(near.w + far.w);//mix(near, far, far.w / (near.w + far.w));
	//return mix(near, far, far.w / (near.w + far.w));
}
void main() {
	//gl_FragColor = T(0,X); return;
	//gl_FragColor = vec4(1., 1., 0., 0.); return;
	vec2 uv = X / Z(1) - .5;

	//gl_FragColor = vec4(mergeNearFar(.5+uv).xyz, 0.); return;

	float amount = .2;
	vec3 color =
		vec3(
			mergeNearFar(.5+uv).r,
			mergeNearFar(.5+uv*(1.+.05*amount)).g,
			mergeNearFar(.5+uv*(1.+.1*amount)).b);

	// FIXME: color.w has wrong semantics
	//color.xyz = pow(color.xyz / (color.xyz + color.w), vec3(1./2.2));
	color /= color + 1.;
	color = pow(color, vec3(1./2.2));

	//gl_FragColor = vec4(color, 1.); return;

	// grain
	//color.xyz -= .03*texture2D(S[0], uv+F[2]*vec2(600.,107.)).y;
	color -= .02*T(0, X*.3+t*vec2(623.,1107.)).y;
	//gl_FragColor = vec4(color, 1.); return;

	// (horrible) vignetting
	//color.xyz *= (1. - smoothstep(.5,.7,length(uv)));
	//uv = 1. - 2. * abs(uv); color*= min(1., 20.*uv.x*uv.y);

	gl_FragColor = vec4(color, 1.);
}
