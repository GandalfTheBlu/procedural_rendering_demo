#version 410

layout(location=0) in mat3 v_TBN;
layout(location=9) in vec2 v_uv;
layout(location=11) in vec3 v_worldPos;

uniform vec3 u_camPos;
uniform mat4 u_VP;

layout(location=0) out vec3 g_worldPos;
layout(location=1) out vec3 g_normal;
layout(location=2) out vec4 g_albedoSpec;

float sdf(vec3 p);

vec3 calcNormal(vec3 p){
	const float h = 0.0001;
	const vec2 k = vec2(1,-1);
	return normalize( k.xyy*sdf(p + k.xyy*h) + 
					  k.yyx*sdf(p + k.yyx*h) + 
					  k.yxy*sdf(p + k.yxy*h) + 
					  k.xxx*sdf(p + k.xxx*h));
}

void marchRay(vec3 pos, vec3 ray, out bool hit, out float t){
	t = 0.;
	hit = false;

	for(int i=0; i<256; i++){
		float r = sdf(pos + ray * t);

		if(r < 0.001){
			hit = true;
			break;
		}

		t += r;
	}
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular);

out float gl_FragDepth;

void main()
{
	vec3 ray = normalize(v_worldPos - u_camPos);
	vec3 surfacePos = vec3(2. * v_uv -1., 0.f);
	mat3 invTBN = transpose(v_TBN);
	vec3 surfaceRay = invTBN * ray;

	bool hit;
	float t;
	marchRay(surfacePos, surfaceRay, hit, t);


	if(!hit){
		discard;
	}

	vec3 hitPos = surfacePos + surfaceRay * t;
	vec3 normal = calcNormal(hitPos);
	vec3 worldNormal = v_TBN * normal;

	vec3 worldPoint = v_worldPos + ray * t;

	vec3 albedo = vec3(1.);
	float specular = 0.5;
	getSurface(hitPos, worldNormal, albedo, specular);

	float zNear = 0.3;
	float zFar = 500.;

	vec4 clipPoint = u_VP * vec4(worldPoint, 1.);

	gl_FragDepth = (clipPoint.z / clipPoint.w + 1.) * 0.5;

	g_worldPos = worldPoint;
	g_normal = worldNormal;
	g_albedoSpec = vec4(albedo, specular);
}