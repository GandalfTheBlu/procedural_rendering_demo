#pragma once
#include "transform.h"
#include "mesh.h"

class Material;

class GraphicsObject : public Object {
public:
	Transform transform;
	Mesh* mesh;
	std::vector<Material*> materials;

	GraphicsObject();
	GraphicsObject(Mesh* mesh, const std::vector<Material*>& materials);
	~GraphicsObject();
};
