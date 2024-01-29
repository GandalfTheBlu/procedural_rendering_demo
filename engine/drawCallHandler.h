#pragma once
#include "material.h"
#include "graphicsObject.h"
#include <unordered_map>

struct RenderNode {
public:
	Material* material;
	size_t subMeshIndex;
	GraphicsObject* graphicsObject;

	RenderNode();
	RenderNode(Material* material, size_t subMeshIndex, GraphicsObject* graphicsObject);
	~RenderNode();
};

class DrawCallHandler {
private:
	std::unordered_map<Shader*, std::vector<RenderNode>> shaderIdToRenderNode;

public:

	DrawCallHandler();
	~DrawCallHandler();

	void Add(GraphicsObject* graphicsObject);
	void Remove(GraphicsObject* graphicsObject);

	void Draw(Camera& camera);
};