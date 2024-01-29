
uniform float u_timer;

float smoothSubtraction(float d1, float d2, float k) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); 
}

float sdf(vec3 p){
	float k = sin(3.1415*0.5*max(u_timer, 0.));
	k = pow(k, 0.4);
	float n0 = sin(p.x*0.4)*sin(p.y*0.5)*sin(p.z*0.45)*0.8;
	float n1 = sin(p.x*1.4)*sin(p.y*2.5)*sin(p.z*1.3)*0.8;
	float n2 = sin(p.x*4.4)*sin(p.y*5.5)*sin(p.z*4.3)*0.2;
	return smoothSubtraction(length(p) - k*k*(4.8 - 0.7*n0), length(p) - k * (4. + n1 + n2), 0.5);
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	float k = sin(3.1415*0.5*max(u_timer, 0.));
	vec3 flame = mix(vec3(1.,0.,0.), vec3(1.,1.,0.), length(hitPos)-4.*k);
	albedo = mix(flame, vec3(0.1), u_timer*u_timer);
}