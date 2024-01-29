#version 430

layout(location=0) in vec2 v_position;


uniform float u_aspect;
uniform float u_tan_half_fovy;
uniform float u_near;
uniform vec3 u_up;
uniform vec3 u_forward;
uniform vec3 u_dirLight;
uniform vec3 u_ambientColor;

out vec3 Color;

vec3 genRay(){
	float half_h = u_near * u_tan_half_fovy;
	float half_w = half_h * u_aspect;
	vec3 right = cross(u_forward, u_up);
	
	vec3 x = right * half_w * v_position.x;
	vec3 y = u_forward * u_near;
	vec3 z = u_up * half_h * v_position.y;

	return normalize(x+y+z);
}

out float gl_FragDepth;

void main(){
	
	gl_FragDepth = 0.9999;

	vec3 ray = genRay();

	float sunDiffuse = max(dot(ray, -u_dirLight), 0.);
	float sunSharp = ceil(sunDiffuse-0.999);

	vec3 col = mix(mix(u_ambientColor, u_ambientColor*1.2, pow(sunDiffuse, 16.)), vec3(1.), sunSharp);

	Color = col;
}