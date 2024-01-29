#pragma once
#include "shader.h"

class ModularShader : public Shader {
public:
	ModularShader();
	ModularShader(ShaderType shaderType, const std::string& modularShaderPath);
	~ModularShader();
};