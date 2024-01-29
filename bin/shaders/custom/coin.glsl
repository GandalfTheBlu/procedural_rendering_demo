
float smoothUnion(float d1, float d2, float k) {
	float h = clamp(0.5 + 0.5*(d2-d1)/k, 0.0, 1.0);
	return mix(d2, d1, h) - k*h*(1.0-h); 
}

float smoothSubtraction(float d1, float d2, float k) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); 
}

float sdf(vec3 p){
	return smoothSubtraction(length(p)-0.7, p.z+0.01, 0.08);
}

uniform float u_time;

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	float glow = 2.*hitPos.z*hitPos.z;
	albedo = mix(vec3(0.5), vec3(0.9, 0.4, 0.0)*abs(sin(u_time*4.)), glow);
}