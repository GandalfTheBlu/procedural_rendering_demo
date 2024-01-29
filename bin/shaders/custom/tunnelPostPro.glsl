#version 430

layout(location=0) in vec2 v_uv;

uniform vec3 u_camPos;

layout(binding=0) uniform sampler2D t_color;
layout(binding=1) uniform sampler2D g_worldPos;

out vec3 Color;

void main(){
	vec3 col = texture(t_color, v_uv).rgb;
	vec3 pos = texture(g_worldPos, v_uv).xyz;
	float dist = dot(pos, pos) == 0. ? 10000. : distance(pos, u_camPos);
	Color = mix(col, vec3(0.2,0.2,0.6), 1.-exp(-0.02*dist));
}