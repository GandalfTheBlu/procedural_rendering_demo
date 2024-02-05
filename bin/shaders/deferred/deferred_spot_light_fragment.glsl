#version 410

uniform sampler2D g_worldPos;
uniform sampler2D g_normal;
uniform sampler2D g_albedoSpec;

layout(location=0) in vec3 v_uvw;

uniform vec3 u_camWorldPos;
uniform vec3 u_lightWorldPos;
uniform vec3 u_lightWorldDirection;
uniform vec3 u_lightColor;
uniform vec3 u_lightAttenuation;
uniform float u_lightRadius;
uniform float u_lightAngle;

uniform int u_stage;

in vec4 gl_FragCoord;

out vec4 Color;

float smoothify(float t){
	return t*t*(3.-2.*t);
}

void main()
{
	if(u_stage == 0){ // just write to stencil buffer
		Color = vec4(0.);
	}
	else{ // perform shading
		vec2 uv = v_uvw.xy / v_uvw.z;
		uv = uv * 0.5 + 0.5;
		
		vec3 worldPos = texture(g_worldPos, uv).rgb;
		vec3 norm = texture(g_normal, uv).rgb;
		vec4 albedoSpec = texture(g_albedoSpec, uv).rgba;
		
		vec3 lightRay = worldPos - u_lightWorldPos;
		float leng = length(lightRay);
		lightRay *= 1./leng;
		
		float quadratic = u_lightAttenuation.x;
		float linear = u_lightAttenuation.y;
		float constant = u_lightAttenuation.z;
		
		float atten = 1. / (constant + linear * leng + quadratic * (leng * leng));
		float startFade = u_lightRadius * 0.9;
		float edgeFade = 1. - smoothify(clamp((leng-startFade)/(u_lightRadius-startFade), 0., 1.));
		atten *= edgeFade;
		
		float endConeFade = cos(u_lightAngle * 0.5 * 0.95);
		float startConeFade = cos(u_lightAngle * 0.5 * 0.85);
		float coneEdgeFade = smoothify(clamp((endConeFade - dot(lightRay, u_lightWorldDirection))/(endConeFade - startConeFade), 0., 1.));
		atten *= coneEdgeFade;
		
		vec3 ray = normalize(worldPos - u_camWorldPos);
		
		float dif = max(0., dot(norm, -lightRay));
		float spec = pow(max(0., dot(reflect(ray, norm), -lightRay)), max(1., albedoSpec.a * 64.));
		vec3 col = (atten * (dif + spec)) * u_lightColor * albedoSpec.rgb;
		
		Color = vec4(col, 1.);
	}
}