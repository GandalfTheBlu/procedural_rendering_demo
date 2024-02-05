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

bool getVoxel(vec3 p);

float marchRay(vec3 origin, vec3 ray, inout vec3 normal, inout vec3 hitIndex){
	float t = 0.;

	vec3 currentIndex = floor(origin);
	vec3 axisSteps = vec3(
		ray.x != 0. ? sqrt(1+(ray.y/ray.x)*(ray.y/ray.x)+(ray.z/ray.x)*(ray.z/ray.x)) : 999999.,
		ray.y != 0. ? sqrt(1+(ray.x/ray.y)*(ray.x/ray.y)+(ray.z/ray.y)*(ray.z/ray.y)) : 999999.,
		ray.z != 0. ? sqrt(1+(ray.x/ray.z)*(ray.x/ray.z)+(ray.y/ray.z)*(ray.y/ray.z)) : 999999.
	);

	vec3 axisDirections = sign(ray);
	vec3 axisDistances = vec3(
		ray.x < 0. ? (origin.x - currentIndex.x) * axisSteps.x : (currentIndex.x + 1 - origin.x) * axisSteps.x,
		ray.y < 0. ? (origin.y - currentIndex.y) * axisSteps.y : (currentIndex.y + 1 - origin.y) * axisSteps.y,
		ray.z < 0. ? (origin.z - currentIndex.z) * axisSteps.z : (currentIndex.z + 1 - origin.z) * axisSteps.z
	);

	vec3 xNormal = vec3(-axisDirections.x,0.,0.);
	vec3 yNormal = vec3(0.,-axisDirections.y,0.);
	vec3 zNormal = vec3(0.,0.,-axisDirections.z);

	for(float i=0.; i<256.; i++){
		if(getVoxel(currentIndex)){
			hitIndex = currentIndex;
			return t;
		}

		if(axisDistances.x < min(axisDistances.y, axisDistances.z)){
			t = axisDistances.x;
			axisDistances.x += axisSteps.x;
			currentIndex.x += axisDirections.x;
			normal = xNormal;
		}
		else if(axisDistances.y < min(axisDistances.x, axisDistances.z)){
			t = axisDistances.y;
			axisDistances.y += axisSteps.y;
			currentIndex.y += axisDirections.y;
			normal = yNormal;
		}
		else{
			t = axisDistances.z;
			axisDistances.z += axisSteps.z;
			currentIndex.z += axisDirections.z;
			normal = zNormal;
		}
	}

	return -1.;
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular);

out float gl_FragDepth;

void main()
{
	vec3 ray = genRay();

	vec3 normal = vec3(0.);
	vec3 hitIndex = vec3(0.);

	float t = marchRay(u_camPos, ray, normal, hitIndex);

	if(t < 0.){
		discard;
	}

	vec3 hitPos = u_camPos + ray * t;

	vec3 albedo = vec3(1.);
	float specular = 0.5;
	getSurface(hitPos, normal, albedo, specular);

	vec4 clipPoint = u_VP *vec4(hitPos, 1.);
	gl_FragDepth = (clipPoint.z / clipPoint.w + 1.) * 0.5;

	g_worldPos = hitPos;
	g_normal = normal;
	g_albedoSpec = vec4(albedo, specular);
}