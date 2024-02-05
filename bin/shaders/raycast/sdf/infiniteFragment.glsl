#version 410

layout(location=0) in vec2 v_position;

uniform float u_aspect;
uniform float u_tan_half_fovy;
uniform float u_near;
uniform vec3 u_up;
uniform vec3 u_forward;
uniform vec3 u_camPos;
uniform mat4 u_VP;

layout(location=0) out vec3 g_worldPos;
layout(location=1) out vec3 g_normal;
layout(location=2) out vec4 g_albedoSpec;

vec3 genRay(){
	float half_h = u_near * u_tan_half_fovy;
	float half_w = half_h * u_aspect;
	vec3 right = cross(u_forward, u_up);
	
	vec3 x = right * half_w * v_position.x;
	vec3 y = u_forward * u_near;
	vec3 z = u_up * half_h * v_position.y;

	return normalize(x+y+z);
}

float sdf(vec3 p);

vec3 calcNormal(vec3 p){
	const float h = 0.0001;
	const vec2 k = vec2(1,-1);
	return normalize( k.xyy*sdf(p + k.xyy*h) + 
					  k.yyx*sdf(p + k.yyx*h) + 
					  k.yxy*sdf(p + k.yxy*h) + 
					  k.xxx*sdf(p + k.xxx*h));
}

float marchRay(vec3 origin, vec3 ray){
	float t = 0.;

	for(float i=0.; i<256.; i++){
		float r = sdf(origin + ray * t);
		if(r < 0.0001){
			return t+r;
		}
		t += r;
	}

	return -1.;
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular);

out float gl_FragDepth;

void main(){
	vec3 ray = genRay();

	float t = marchRay(u_camPos, ray);

	if(t < 0.){
		discard;
	}

	vec3 hitPos = u_camPos + ray * t;
	vec3 normal = calcNormal(hitPos);

	vec3 albedo = vec3(1.);
	float specular = 0.5;
	getSurface(hitPos.xyz, normal, albedo, specular);

	vec4 clipPoint = u_VP *vec4(hitPos, 1.);
	gl_FragDepth = (clipPoint.z / clipPoint.w + 1.) * 0.5;

	g_worldPos = hitPos;
	g_normal = normal;
	g_albedoSpec = vec4(albedo, specular);
}