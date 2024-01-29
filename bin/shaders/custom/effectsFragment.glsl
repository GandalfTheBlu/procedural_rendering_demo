#version 430

layout(location=0) in vec2 v_uv;

layout(binding=0) uniform sampler2D t_color;

out vec3 Color;

void main(){
	vec3 col = texture(t_color, v_uv).rgb;
	Color = pow(col * vec3(1.,0.8,0.6), vec3(0.95));
}