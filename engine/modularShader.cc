#include "modularShader.h"

ModularShader::ModularShader(){}
ModularShader::ModularShader(ShaderType shaderType, const std::string& modularShaderPath) {
	this->type = shaderType;

	std::string vertexShaderPath;
	std::string fragmentShaderPath;

	switch (shaderType) {
	case ShaderType::BoundSDF:
		vertexShaderPath = "shaders/raycast/commonVertex.glsl";
		fragmentShaderPath = "shaders/raycast/sdf/boundFragment.glsl";
		break;
	case ShaderType::InfiniteSDF:
		vertexShaderPath = "shaders/raycast/commonVertex.glsl";
		fragmentShaderPath = "shaders/raycast/sdf/infiniteFragment.glsl";
		break;
	case ShaderType::ProceduralVoxels:
		vertexShaderPath = "shaders/raycast/commonVertex.glsl";
		fragmentShaderPath = "shaders/raycast/voxels/proceduralFragment.glsl";
		break;
	case ShaderType::VoxelData:
		vertexShaderPath = "shaders/raycast/commonVertex.glsl";
		fragmentShaderPath = "shaders/raycast/voxels/dataFragment.glsl";
		break;
	case ShaderType::SDFSurface:
		vertexShaderPath = "shaders/mesh/sdf/surfaceVertex.glsl";
		fragmentShaderPath = "shaders/mesh/sdf/surfaceFragment.glsl";
		break;
	case ShaderType::SDFVolume:
		vertexShaderPath = "shaders/mesh/sdf/volumeVertex.glsl";
		fragmentShaderPath = "shaders/mesh/sdf/volumeFragment.glsl";
		break;
	}

	std::string vertexShaderContent;
	std::string fragmentShaderContent;
	std::string modularShaderContent;

	bool loadSuccess =
		this->ReadFile(vertexShaderPath, vertexShaderContent) &&
		this->ReadFile(fragmentShaderPath, fragmentShaderContent) &&
		this->ReadFile(modularShaderPath, modularShaderContent);

	if (!loadSuccess) { return; }

	std::string endIncludeStr = "//[end_include]";
	size_t endInclude = modularShaderContent.find(endIncludeStr);
	
	if (endInclude != std::string::npos) {
		endInclude += endIncludeStr.length();
	}
	else {
		endInclude = 0;
	}

	fragmentShaderContent += modularShaderContent.substr(endInclude, std::string::npos);

	this->CreateProgram(vertexShaderContent, fragmentShaderContent);
}
ModularShader::~ModularShader(){}