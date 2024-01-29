
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

vec3 repeatXY(vec3 p, vec2 spacing)
{
	return vec3(p.xy - spacing * round(p.xy / spacing), p.z);
}

float brickSDF(vec3 p)
{
	float plane = p.z + 0.04;
	float cutout = box(p-vec3(0.,0.,0.2), vec3(0.7,0.7,0.5))-0.08;
	float bricksLayer1 = box(repeatXY(p + vec3(0., 0., 0.55), vec2(0.5, 0.5)), vec3(0.2, 0.1, 0.2));
	float bricksLayer2 = box(repeatXY(p + vec3(0.25, 0.25, 0.55), vec2(0.5, 0.5)), vec3(0.2, 0.1, 0.2));
	return smoothUnion(smoothSubtraction(cutout, plane, 0.1), min(bricksLayer1, bricksLayer2), 0.05);
}

void brickSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular)
{
	float bricksLayer1 = box(repeatXY(hitPos + vec3(0., 0., 0.55), vec2(0.5, 0.5)), vec3(0.2, 0.1, 0.2));
	float bricksLayer2 = box(repeatXY(hitPos + vec3(0.25, 0.25, 0.55), vec2(0.5, 0.5)), vec3(0.2, 0.1, 0.2));
	float brickDist = min(bricksLayer1, bricksLayer2);

	if(brickDist < 0.02)
	{
		albedo = vec3(0.8, 0.4, 0.1);
	}
	else
	{
		albedo = vec3(0.2);
	}
}

float sdf(vec3 p){
	return brickSDF(p);
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	brickSurface(hitPos, normal, albedo, specular);
}