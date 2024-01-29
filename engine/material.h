#pragma once
#include "modularShader.h"
#include "camera.h"

class GraphicsObject;

class Material : public Object {
public:
	enum class RenderType {
		Deferred,
		Forward,
		UI
	};

	Shader* shader;
	void (*ApplyModularShaderSettings)(GraphicsObject*, Shader*, Camera&);
	void (*ApplyCustomShaderSettings)(GraphicsObject*, Shader*, Camera&);
	void (*CleanUpAfterDraw)();
	RenderType renderType;

	Material();
	Material(Shader* shader, void(*ApplyCustomShaderSettings)(GraphicsObject*, Shader*, Camera&) = nullptr);
	~Material();
};