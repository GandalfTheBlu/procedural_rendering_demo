#version 410

layout(location=0) in vec2 a_position;

layout(location=0) out vec2 v_uv;

void main()
{
	v_uv = a_position * 0.5 + 0.5;
	gl_Position = vec4(a_position, 0., 1.);
}