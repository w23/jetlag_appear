uniform vec2 uv2_resolution;
uniform sampler2D us2_frame;
uniform vec2 uv2_frame_resolution;
uniform sampler2D us2_rand;
uniform vec2 uv2_rand_resolution;

float hash(float n) { return texture2D(us2_rand, vec2(n*17.,n*53.)/uv2_rand_resolution).x; }
vec2 hash2(float n) { return texture2D(us2_rand, vec2(n*17.,n*53.)/uv2_rand_resolution).yz; }
float hash(vec2 n) { return texture2D(us2_rand, n*17./uv2_rand_resolution).y; }
float noise(vec2 v) { return texture2D(us2_rand, (v+.5)/uv2_rand_resolution).z; }
float noise(float v) { return noise(vec2(v)); }

const float R = 40.;
const int N = 64;

vec3 bokeh_wannabe(vec2 p) {
	vec3 color = vec3(0.);
	for (int i = 0; i < N; ++i) {
		vec2 s = R * (hash2(float(i)) - .5);
		color += texture2D(us2_frame, p + s / uv2_frame_resolution).xyz;
	}
	return color / float(N);
}

void main() {
	vec2 uv = gl_FragCoord.xy / uv2_resolution;
	vec3 color = texture2D(us2_frame, uv).xyz;
	gl_FragColor = vec4(pow(color + bokeh_wannabe(uv), vec3(1./2.2)), 1.);
}
