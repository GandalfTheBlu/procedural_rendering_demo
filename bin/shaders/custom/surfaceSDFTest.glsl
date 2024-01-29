
float box(vec3 p, vec3 b){
	vec3 q = abs(p) - b;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float smoothSubtraction(float d1, float d2, float k) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); 
}

float sdf(vec3 p){
	vec3 q = p;
	q.xy *= 10.;
	float stripesX = 1.-pow(1.-min(fract(q.x), fract(1.-q.x)), 8.);
	float stripesY = 1.-pow(1.-min(fract(q.y), fract(1.-q.y)), 8.);
	float stripes = min(stripesX, stripesY);

	float plane = p.z + 0.01 - stripes * 0.006;

	float b1 = box(p-vec3(0.,0.,0.2), vec3(0.6,0.6,0.5))-0.08;
	
	return smoothSubtraction(b1, plane, 0.2);
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	albedo = vec3(-hitPos.z*2., 0.5, 0.5);
}