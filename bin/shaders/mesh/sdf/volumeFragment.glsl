#version 410

layout(location=0) in vec3 v_localPos;

uniform vec3 u_camLocalPos;
uniform mat4 u_M;
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

void main(){
	vec3 origin = v_localPos;
	vec3 ray = normalize(origin - u_camLocalPos);

	bool hit;
	float t;
	marchRay(origin, ray, hit, t);

	if(!hit){
		discard;
	}

	vec3 hitPos = origin + ray * t;
	vec3 worldNormal = normalize((u_M * vec4(calcNormal(hitPos), 0.)).xyz);

	vec3 albedo = vec3(1.);
	float specular = 0.5;
	getSurface(hitPos, worldNormal, albedo, specular);

	vec4 worldPos = u_M * vec4(hitPos, 1.);

	vec4 clipPoint = u_VP * worldPos;
	gl_FragDepth = (clipPoint.z / clipPoint.w + 1.) * 0.5;

	g_worldPos = worldPos.xyz;
	g_normal = worldNormal;
	g_albedoSpec = vec4(albedo, specular);
}