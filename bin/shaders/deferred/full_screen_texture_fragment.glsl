#version 430

layout(binding=0) uniform sampler2D u_texture;

layout(location=0) in vec2 v_uv;

out vec3 Color;

void main()
{
	Color = texture(u_texture, v_uv).rgb;
}