#version 410

layout(location=0) in vec2 v_position;

uniform float u_aspect;
uniform float u_tan_half_fovy;
uniform float u_near;
uniform vec3 u_up;
uniform vec3 u_forward;
uniform vec3 u_camPos;
uniform mat4 u_VP;
uniform vec3 u_volumeSize;
uniform vec3 u_volumePos;

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

vec2 rayBoxDist(vec3 containerMin, vec3 containerMax, vec3 rayOrigin, vec3 rayDir) {
    vec3 t0 = (containerMin - rayOrigin) / rayDir;
    vec3 t1 = (containerMax - rayOrigin) / rayDir;
    vec3 tMin = min(t0, t1);
    vec3 tMax = max(t0, t1);

    float distA = max(max(tMin.x, tMin.y), tMin.z);
    float distB = min(tMax.x, min(tMax.y, tMax.z));

    float distToBox = max(0., distA);
    float distInsideBox = max(0., distB - distToBox);
    return vec2(distToBox, distInsideBox);
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
	vec2 boxDist = rayBoxDist(-u_volumeSize*0.5, 0.5*u_volumeSize, origin, ray);
	
	if(boxDist.y == 0.){
		return -1.;
	}

	float t = boxDist.x;
	float tInBox = 0.;

	for(float i=0.; i<256.; i++){
		float r = sdf(origin + ray * t);
		t += r;
		tInBox += r;
		if(tInBox > boxDist.y){
			return -1.;
		}
		if(r < 0.0001){
			return t+r;
		}
	}

	return -1.;
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular);

out float gl_FragDepth;

void main(){
	vec3 ray = genRay();
	vec3 translation = -u_volumePos - u_volumeSize * 0.5;
	vec3 origin = u_camPos + translation;
	float t = marchRay(origin, ray);

	if(t < 0.){
		discard;
	}

	vec3 localHitPos = origin + ray * t;
	vec3 normal = calcNormal(localHitPos);

	vec3 albedo = vec3(1.);
	float specular = 0.5;
	getSurface(localHitPos, normal, albedo, specular);

	vec3 worldPos = localHitPos - translation;

	vec4 clipPoint = u_VP *vec4(worldPos, 1.);
	gl_FragDepth = (clipPoint.z / clipPoint.w + 1.) * 0.5;

	g_worldPos = worldPos;
	g_normal = normal;
	g_albedoSpec = vec4(albedo, specular);
}