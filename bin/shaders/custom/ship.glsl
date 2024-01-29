
float smoothUnion(float d1, float d2, float k) {
	float h = clamp(0.5 + 0.5*(d2-d1)/k, 0.0, 1.0);
	return mix(d2, d1, h) - k*h*(1.0-h); 
}

float smoothSubtraction(float d1, float d2, float k) {
    float h = clamp( 0.5 - 0.5*(d2+d1)/k, 0.0, 1.0 );
    return mix( d2, -d1, h ) + k*h*(1.0-h); 
}

uniform float u_time;
uniform float u_speed;

float sdf(vec3 p){
	vec3 q1 = p;
	q1.y *= 0.2;
	float mainBody = max(length(q1)-0.5, length(p-vec3(0.,1.5,0.))-1.5);

	vec3 q2 = abs(p) - vec3(0.6,0.,0.);
	q2.y *= 0.35;
	float t = u_time * 35.;
	float thrusters = length(q2)-0.3 - 0.002*min(u_speed, 35.)*max(0., -p.y+0.2)*sin(20.*q2.x+t)*sin(20.*q2.z+t);

	vec3 q3 = p - vec3(0., 0.6, 0.6);
	q3.y *= 0.5;
	float seat = length(q3)-0.3;

	return smoothUnion(smoothSubtraction(seat, mainBody, 0.1), thrusters, 0.1);
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	float flames = max(0., -hitPos.y+0.2)*(1.-exp(-0.02*u_speed));
	float tip = max(0., hitPos.y-0.8);
	tip *= tip;
	vec3 flameColor = mix(vec3(0.8,0.4,0.), vec3(0.2,0.4,0.9), clamp((u_speed-35.)/(60.-35.), 0., 1.));
	albedo = vec3(0.5+0.1*tip, 0.5, 0.5) + flames*flameColor;
}