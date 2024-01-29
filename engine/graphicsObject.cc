#include "graphicsObject.h"

GraphicsObject::GraphicsObject() {
	this->mesh = nullptr;
}
GraphicsObject::GraphicsObject(Mesh* mesh, const std::vector<Material*>& materials) {
	this->mesh = mesh;
	this->materials = materials;
}
GraphicsObject::~GraphicsObject() {}