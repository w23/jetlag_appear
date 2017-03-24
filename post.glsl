uniform vec2 uv2_resolution;
uniform sampler2D us2_frame;
uniform vec2 uv2_frame_resolution;
uniform sampler2D us2_rand;
uniform vec2 uv2_rand_resolution;
uniform float uf_time;

float hash(float n) { return texture2D(us2_rand, vec2(n*17.,n*53.)/uv2_rand_resolution).x; }
vec2 hash2(float n) { return texture2D(us2_rand, vec2(n*17.,n*53.)/uv2_rand_resolution).yz; }
float hash(vec2 n) { return texture2D(us2_rand, n*17./uv2_rand_resolution).y; }
vec2 hash2(vec2 n) { return texture2D(us2_rand, n*17./uv2_rand_resolution).zw; }
float noise(vec2 v) { return texture2D(us2_rand, (v+.5)/uv2_rand_resolution).z; }
float noise(float v) { return noise(vec2(v)); }


vec3 bloom(vec2 p) {
	vec3 color = vec3(0.);
#if 0
	const float R = 100.;
	const int N = 128;
	for (int i = 0; i < N; ++i) {
		vec2 s = R * (hash2((vec2(float(i)+uf_time*0.)+p)*100.) - .5);
		color += texture2D(us2_frame, p + s / uv2_frame_resolution).xyz;
	}
#else
	const int Ns = 16, N = Ns * Ns;
	for (int y = 0; y < Ns; ++y)
		for (int x = 0; x < Ns; ++x) {
			vec2 o = 3. * vec2(ivec2(x,y) - Ns / 2);
			color += texture2D(us2_frame, p + o / uv2_frame_resolution).xyz; 
		}
#endif
	return pow(color / float(N), vec3(2.));
}

void main() {
	vec2 uv = gl_FragCoord.xy / uv2_resolution;
	vec3 srccolor = texture2D(us2_frame, uv).xyz;
	//gl_FragColor = vec4(srccolor, 1.); return;
	vec3 color = srccolor*1. + 3.*bloom(uv);
	gl_FragColor = vec4(pow(color, vec3(1./2.2)), 1.);
}
