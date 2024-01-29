
float box(vec3 p, vec3 b){
	vec3 q = abs(p) - b;
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float smoothUnion(float d1, float d2, float k) {
	float h = clamp(0.5 + 0.5*(d2-d1)/k, 0.0, 1.0);
	return mix(d2, d1, h) - k*h*(1.0-h); 
}

float smoothSubtraction(float d1, float d2, float k) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); 
}

vec3 fold(vec3 p, vec3 normal)
{
	return p - 2. * min(0., dot(p, normal)) * normal;
}

float moltenRockSDF(vec3 p)
{
	float r = 0.2;
	float d = length(p) - r;
	const vec3 n1 = normalize(vec3(1., 0.5, 0.));
	const vec3 n2 = normalize(vec3(0.7, -1., 0.));
	float flip = 1.;

	for(int i=0; i<8; i++)
	{
		p = fold(p, n1);
		p = fold(p, n2);
		p.x -= r;
		float y = p.y;
		p.y = p.x;
		p.x = y;
		r *= 0.8;
		float sd = length(p) - r;
		float d2 = flip * max(d, sd) + (1. - flip) * min(d, sd);
		d = (d + d2) * 0.5;
		flip = 1. - flip;
	}

	return d;
};

void moltenRockSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	float depth = (0.25 + hitPos.z) / 0.2;
	albedo = mix(vec3(1., 0.3, 0.), vec3(0.3, 0.3, 0.3), depth);
}

float sdf(vec3 p){
	float plane = p.z + 0.01;
	float cutout = box(p-vec3(0.,0.,0.2), vec3(0.6,0.6,0.5))-0.08;
	float detail = moltenRockSDF(p + vec3(0., 0., 0.1));
	
	return smoothUnion(smoothSubtraction(cutout, plane, 0.2), detail, 0.23);
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	moltenRockSurface(hitPos, normal, albedo, specular);
}