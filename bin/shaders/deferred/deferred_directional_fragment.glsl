#version 410

uniform sampler2D g_worldPos;
uniform sampler2D g_normal;
uniform sampler2D g_albedoSpec;

layout(location=0) in vec2 v_uv;

uniform vec3 u_camWorldPos;
uniform vec3 u_ambientColor;
uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform float u_intensity;

out vec3 Color;

void main()
{
	vec3 worldPos = texture(g_worldPos, v_uv).rgb;
	vec3 norm = texture(g_normal, v_uv).rgb;
	vec4 albedoSpec = texture(g_albedoSpec, v_uv).rgba;
	
	vec3 ray = normalize(worldPos - u_camWorldPos);
	
	float dif = max(0., dot(norm, -u_lightDirection));
	float spec = pow(max(0., dot(reflect(ray, norm), -u_lightDirection)), max(1., albedoSpec.a*64.));
	vec3 col = ((u_intensity * (spec + dif))  * u_lightColor + u_ambientColor) * albedoSpec.rgb;
	
	Color = col;
}