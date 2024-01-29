#version 430

layout(location=0) in vec3 a_position;

uniform mat4 u_MVP;

layout(location=0) out vec3 v_localPos;

void main()
{
	gl_Position = u_MVP * vec4(a_position, 1.);
	v_localPos = a_position;
}