uniform vec3 u_camPos;
//[end_include]

uniform float u_time;

float mod1(float a, float b){
	return a - b*floor(a/b);
}

vec3 mod3(vec3 p, vec3 q){
	return vec3(mod1(p.x,q.x), mod1(p.y,q.y), mod1(p.z,q.z));
}

vec3 rep(vec3 p, vec3 q){
	return mod3(p+q*0.5, q)-q*0.5;
}

vec3 rotX(vec3 p, float a){
	float r = length(p.yz);
	float angle = atan(p.z, p.y) + a;
	return vec3(p.x, r*cos(angle), r*sin(angle));
}

float sdCapsule(vec3 p, vec3 a, vec3 b, float r){
  vec3 pa = p - a, ba = b - a;
  float h = clamp(dot(pa,ba)/dot(ba,ba), 0.0, 1.0);
  return length(pa - ba*h) - r;
}

uniform vec3 u_shipPos;

bool getVoxel(vec3 p){
	float s = 40.;

	vec3 p0 = p;

	p = rotX(p.yxz, 0.05*u_time).yxz;

	vec3 q1 = rotX(p.yxz, p.y*0.05);
	vec3 q2 = rotX(p.yxz, p.x*0.05);
	float t = sin(u_time*0.1);
	t *= t;
	vec3 q = rep(mix(q1, q2, t), vec3(s));

	float c = 0.5;
	float dr = 0.9*pow(abs(sin(u_time*0.1)), 7.) + 1.5*sin(p.x*c)*sin(p.y*c)*sin(p.z*c);
	dr *= dr * 0.1;
	bool sphere = length(q) - s*(0.6+dr) < 0.;

	bool capsule = sdCapsule(p0, u_camPos, u_shipPos, 3.) < 0.;

	return !sphere && !capsule;
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	specular = 0.1;
	albedo = vec3(0.2) + 0.1*sin(hitPos*0.1);
	normal = normalize(normal + 0.1*sin(hitPos*10.));
}