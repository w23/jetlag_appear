float t = $(float time);
uniform sampler2D S[1];

vec3 E = vec3(0.,.001,1.);

vec4 noise(vec2 v) { return texture2D(S[0], (v + .5)/textureSize(S[0],0)); }

vec4 cseed;
float world(vec3 p) {
	float d = p.y;
	vec2 C = floor(p.xz), TC = C;
	float min1 = 10., min2 = 10.;
	for (float x = -1.; x <= 1.; x+=1.)
	for (float y = -1.; y <= 1.; y+=1.) {
		vec2 cn = C + vec2(x, y), c = cn + .5;
		cseed = noise(cn);
		c += (cseed.yz - .5) * .7;
		//float dist = length(c - p.xz);
		float dist = abs(c.x - p.x) + abs(c.y - p.z);
		if (dist < min1) {
			min2 = min1;
			min1 = dist;
			TC = cn;
		} else if (dist < min2)
			min2 = dist;
	}

	float vd = min2 - min1;

	cseed = noise(TC);
	d = min(d, max(p.y - cseed.x, (.3 - vd) * .5));
	return d;
}

vec3 normal(vec3 p) {
	return normalize(vec3(
		world(p+E.yxx), world(p+E.xyx), world(p+E.xxy)) - world(p));
}

float march(vec3 o, vec3 d, float l, float maxl) {
	float mind=10., minl = l;
	for (int i = 0; i < 99; ++i) {
		float d = world(o + d * l);
		l += d * .9;
		if (d < .001 * l || l > maxl) return l;

		//if (l > maxl) break;

		if (d < mind) {
			mind = d; minl = l;
		}

	}
	return minl;
}

vec3 O, D, N, P;
vec4 m_diffk = vec4(1.,1.,1.,.5);
float m_shine = 10.;

float dirlight(vec3 ld) {
	return mix(
		max(0., dot(N, ld)),
		max(0., pow(dot(N, normalize(ld - D)), m_shine) * (m_shine + 8.) / 24.),
		m_diffk.w);
}

const float EPS = .01, FAR = 5000., SKY = FAR, GS = 32.;
vec3 sundir = normalize(vec3(1.,sin(t*.1),1.));
const float I=10.,g=.76,g2=g*g,R0=6360e3,R1=6420e3;
const vec3 C=vec3(0.,-R0,0.),Km=vec3(21e-6),Kr=vec3(5.8,13.5,33.1)*1e-6;
vec2 SD(vec3 O) {
  float h = R0 - length(O - C);
  return vec2(exp(h / 120.), exp(h / 8000.));
}
float ER(vec3 o, vec3 d, float r) {
  o -= C;
  float
    b = dot(o, d),
    c = dot(o, o) - r * r,
    e = b * b - c,
    p, q;
  if (e < 0.) return -1.;
  e = sqrt(e);
  p = -b - e,
  q = -b + e;
  return (p >= 0.) ? p : q;
}

vec2 Id(vec3 O, vec3 D, float L) {
  float dx, bdx = L / 16., l = 0.;
  vec3 p = O;
  vec2 mr = vec2(0.);
  for (int i = 0; i < 16; ++i) {
    dx = bdx * (1. + noise(p.xz * 2e3).w);
    p = O + D * (l + dx);
    mr += SD(p) * dx;
    l += bdx;
  }
  return mr;
}
vec3 scatter(vec3 O, vec3 D, float L, vec3 cc) {
	float c = max(0., dot(D, sundir)),
  Pr = .0596831 * (1. + c * c),
  Pm = .1193662 * (1. - g2) * (1. + c * c) / ((2. + g2) * pow(1. + g2 - 2.*g*c, 1.5)),
  dm = 0.,
  dr = 0.,
  ll = 0.,
  bdx = L / 32., sL;

  vec3 im = vec3(0.), ir = vec3(0.), p = O, Is;
  vec2 mr;
  for (int i = 0; i < 32; ++i) {
    float dx = bdx * (1. + noise(p.xz * 2e3).w);
    p = O + D * (ll + dx);
    ll += bdx;
    mr = SD(p) * dx;
    dm += mr.x;
    dr += mr.y;

    // if shadowed by terrain
    //float Lt = TT(p, sundir);
    //if (Lt < Far) continue;

    sL = ER(p,sundir,R1);

    vec2 dd = Id(p, sundir, sL);

    Is = exp(-(
        Km * (dd.x + dm) +
        Kr * (dd.y + dr)
      ));
    im += mr.x * Is;
    ir += mr.y * Is;
  }

  return max(vec3(0.),I * (
      Km * Pm * im +
      Kr * Pr * ir)
    + cc * exp(-(Km * dm + Kr * dr)));
}

void main() {
	const vec2 res = vec2(1280.,720.);
	vec2 uv = gl_FragCoord.xy/res * 2. - 1.; uv.x *= res.x / res.y;

	vec3 at = vec3(0.);

	O = $(vec3 cam_pos); //10. * vec3(sin(t*.1), 0., cos(t*.1)) + vec3(0., 3. + 2. * sin(t*.2), 0.);
	D = $(vec3 cam_dir);//normalize(at - O);
	vec3 x = normalize(cross(vec3(0.,1.,0.), D));
	D = mat3(x, cross(D, x), D) * normalize(vec3(uv, 1.));
	vec3 color = vec3(0.);

	const float md = 50.;
	float l = march(O, D, 0., md);
	if (l < md) {
		P = O + D * l;
		N = normal(P);

		float flr = max(0., sin(P.y*40.));
		m_shine = 10. + flr * 90.;
		color = (vec3(1.) + .04 *cseed.xyz) * (.5 + .5 * flr);
		color *= dirlight(sundir);
	} else {
		color = scatter(O, D, ER(O, D, R1), vec3(0.));
	}

	gl_FragColor = vec4(sqrt(color),0.);
}
