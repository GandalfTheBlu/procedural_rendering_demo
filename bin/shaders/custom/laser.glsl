
float mod1(float a, float b){
	return a - b*floor(a/b);
}

float rep(float a, float b){
	return mod1(a+b*0.5, b)-b*0.5;
}

uniform float u_time;
uniform float u_isShooting;

float sdf(vec3 p){

	p.y = rep(p.y - 60.*u_time, 6.);
	p.xz *= 2.;
	return length(p) - 0.2*u_isShooting;
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	float rg = 0.4-6.*min(0.,rep(hitPos.y - 60.*u_time, 6.));
	albedo = vec3(rg, rg, 0.);
}