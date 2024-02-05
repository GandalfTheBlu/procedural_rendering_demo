#version 430

layout(location=0) in vec2 v_uv;

uniform sampler2D t_color;

out vec3 Color;

void main(){
	vec3 col = texture(t_color, v_uv).rgb;
	Color = 1. - col;
}