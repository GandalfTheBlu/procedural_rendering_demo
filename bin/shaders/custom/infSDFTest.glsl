
uniform float u_time;


float smin(float d1, float d2, float k) {
    float h = clamp(0.5 + 0.5*(d2-d1)/k, 0.0, 1.0);
    return mix(d2, d1, h) - k*h*(1.0-h); 
}

float sdf(vec3 p){
	vec2 f = fract(p.xy);
	float edge = 1. - f.x * (1.-f.x) * f.y * (1.-f.y);
	edge = pow(edge, 16.);
	
	float waterPuffs = p.z + 2.2 + 0.1*sin(p.x + u_time) + 0.15*edge;

	vec3 q = p-vec3(25.,20.,-2.2+6.*sin(u_time));
	vec3 n = sin(q*10.);
	float sphere = length(q) - 6. - 0.05*n.x*n.y*n.z;

	return smin(sphere, waterPuffs, 2.8);
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	
	float sphere = exp(-(length(hitPos-vec3(25.,20.,-2.2+6.*sin(u_time))) - 6.));

	albedo = mix(vec3(0.2,0.25,1.), vec3(0.6,0.6,0.8), sphere);
	specular = mix(1., 0.5, sphere);
}