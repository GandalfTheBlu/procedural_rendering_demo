#version 410

layout(location=0) in vec3 a_position;

layout(location=0) out vec3 v_uvw;

uniform mat4 u_MVP;

void main()
{
	vec4 pos = u_MVP * vec4(a_position, 1.);
	v_uvw = pos.xyw;
	gl_Position = pos;
}