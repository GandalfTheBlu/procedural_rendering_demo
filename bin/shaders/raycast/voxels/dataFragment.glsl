#version 410

layout(location=0) in vec2 v_position;

uniform usampler3D voxelTexture;

uniform float u_aspect;
uniform float u_tan_half_fovy;
uniform float u_near;
uniform vec3 u_up;
uniform vec3 u_forward;
uniform vec3 u_camPos;
uniform mat4 u_VP;
uniform mat4 u_M;
uniform mat4 u_invM;

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

uint getVoxel(vec3 p){
	ivec3 s = textureSize(voxelTexture, 0);

	if(p.x < 0. || p.x >= s.x || p.y < 0. || p.y >= s.y || p.z < 0. || p.z >= s.z){
		return 0;
	}

	p /= s;

	return texture(voxelTexture, p).r;
}

uint getSubVoxel(ivec3 subP, uint voxelValue){
	uint index = subP.z*4 + subP.y*2 + subP.x;
	return 1 & (voxelValue >> index);
}

bool isInBounds(ivec3 subP){
	return subP.x >= 0 && subP.x < 2 &&
			subP.y >= 0 && subP.y < 2 &&
			subP.z >= 0 && subP.z < 2;
}

float marchSubVoxel(vec3 voxelIndex, vec3 currentPos, vec3 axisSteps, ivec3 axisDirections, uint voxelValue, inout vec3 normal){
	float t = 0.;
	bool hit = false;
	
	vec3 currentSubPos = clamp((currentPos - voxelIndex)*2., vec3(0.), vec3(1.9999));
	ivec3 currentSubIndex = ivec3(currentSubPos);
	
	vec3 axisDistances = vec3(
		axisDirections.x < 0 ? (currentSubPos.x - float(currentSubIndex.x)) * axisSteps.x : (float(currentSubIndex.x)+1. - currentSubPos.x) * axisSteps.x,
		axisDirections.y < 0 ? (currentSubPos.y - float(currentSubIndex.y)) * axisSteps.y : (float(currentSubIndex.y)+1. - currentSubPos.y) * axisSteps.y,
		axisDirections.z < 0 ? (currentSubPos.z - float(currentSubIndex.z)) * axisSteps.z : (float(currentSubIndex.z)+1. - currentSubPos.z) * axisSteps.z
	);

	vec3 xNormal = vec3(-axisDirections.x,0.,0.);
	vec3 yNormal = vec3(0.,-axisDirections.y,0.);
	vec3 zNormal = vec3(0.,0.,-axisDirections.z);

	for(float i=0.; i<8.; i++){
		if(!isInBounds(currentSubIndex)){
			break;
		}
		if(getSubVoxel(currentSubIndex, voxelValue) != 0){
			hit = true;
			break;
		}

		if(axisDistances.x < min(axisDistances.y, axisDistances.z)){
			t = axisDistances.x;
			axisDistances.x += axisSteps.x;
			currentSubIndex.x += axisDirections.x;
			normal = xNormal;
		}
		else if(axisDistances.y < min(axisDistances.x, axisDistances.z)){
			t = axisDistances.y;
			axisDistances.y += axisSteps.y;
			currentSubIndex.y += axisDirections.y;
			normal = yNormal;
		}
		else{
			t = axisDistances.z;
			axisDistances.z += axisSteps.z;
			currentSubIndex.z += axisDirections.z;
			normal = zNormal;
		}
	}

	return hit ? t * 0.5 : -1.;
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

float marchRay(vec3 origin, vec3 ray, inout vec3 normal){
	ivec3 s = textureSize(voxelTexture, 0);

	origin += s * 0.5;

	vec2 boxDist = rayBoxDist(vec3(0.), s, origin, ray);
	
	if(boxDist.y == 0.){
		return -1.;
	}

	float t = 0.;
	
	origin += ray * (boxDist.x * 0.999);

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
		if(t >= boxDist.x+boxDist.y){
			return -1.;
		}

		uint voxelValue = getVoxel(currentIndex);
		if(voxelValue != 0){
			float subT = marchSubVoxel(currentIndex, origin+ray*t, axisSteps, ivec3(axisDirections), voxelValue, normal);
			if(subT >= 0.){
				return t + subT + boxDist.x;
			}
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

out float gl_FragDepth;

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular);

void main()
{
	vec3 ray = genRay();

	vec3 localRay = normalize((u_invM * vec4(ray, 0.)).xyz);
	vec3 localCamPos = (u_invM * vec4(u_camPos, 1.)).xyz;
	vec3 localNormal = vec3(0.);

	float localT = marchRay(localCamPos, localRay, localNormal);

	if(localT < 0.){
		discard;
	}

	vec3 normal = normalize((u_M * vec4(localNormal, 0.)).xyz);
	vec3 localHitPos = localCamPos + localRay * localT;
	vec4 hitPos = u_M * vec4(localHitPos, 1.);
	
	vec3 albedo = vec3(1.);
	float specular = 0.5;
	getSurface(hitPos.xyz, normal, albedo, specular);

	vec4 clipPoint = u_VP * hitPos;
	gl_FragDepth = (clipPoint.z / clipPoint.w + 1.) * 0.5;

	g_worldPos = hitPos.xyz;
	g_normal = normal;
	g_albedoSpec = vec4(albedo, specular);
}