
float sdf(vec3 p){
	return length(p) - 0.4;
}

void getSurface(vec3 hitPos, inout vec3 normal, inout vec3 albedo, inout float specular){
	albedo = vec3(0.3, 0.3, 0.3);
	specular = 0.1;
}