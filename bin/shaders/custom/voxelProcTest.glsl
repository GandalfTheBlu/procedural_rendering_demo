uniform vec3 u_camPos;
//[end_include]

bool getVoxel(vec3 p){
	float s = 0.4;
	float n1 = 1.2*sin(p.x*s+p.y)*sin(p.y*s+p.x);
	float n2 = 10.*sin(2.4+p.x*s*0.1)*sin(p.y*s*0.1);
	return (p.z + 5. - n1 - n2) < -0.2;
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	
	vec3 tex = vec3(0.);
	vec3 base = vec3(0.);

	if(abs(normal.z) > 0.5){
		vec2 uv = fract(hitPos.xy);
		
		float dirt = 0.5 + 0.5 * sin(uv.x * 31.415) * sin(uv.y * 31.415);

		tex = vec3(0.4, 0.6-0.3*dirt, 0.2);
		base = vec3(0.4, 0.45, 0.2);
	}
	else{
		vec2 uv = abs(normal.y) > 0.5 ? fract(hitPos.xz) : fract(hitPos.yz);

		float grass = uv.y-0.05*sin(uv.x*31.415) > 0.8 ? 1. : 0.;

		tex = vec3(0.4, 0.3+0.3*grass, 0.2);
		base = vec3(0.4, 0.3, 0.2);
	}

	vec3 ray = normalize(hitPos - u_camPos);
	float blend = 1.-pow(max(-dot(ray, normal), 0.),0.5)*exp(-0.05*distance(u_camPos, hitPos));
	albedo = mix(tex, base, blend);

	specular = 0.2;
}