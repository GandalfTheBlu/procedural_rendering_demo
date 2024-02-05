#version 410

layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec3 a_tangent;
layout(location=3) in vec2 a_uv;

uniform mat4 u_M;
uniform mat4 u_MVP;

layout(location=0) out mat3 v_TBN;
layout(location=9) out vec2 v_uv;
layout(location=11) out vec3 v_worldPos;

void main()
{
	gl_Position = u_MVP * vec4(a_position, 1.);

	vec3 normal = (u_M * vec4(a_normal, 0.)).xyz;
	vec3 tangent = (u_M * vec4(a_tangent, 0.)).xyz;
	vec3 bitangent = cross(normal, tangent);

	v_worldPos = (u_M * vec4(a_position, 1.)).xyz; 
	v_TBN = mat3(tangent, bitangent, normal);
	v_uv = a_uv;
}