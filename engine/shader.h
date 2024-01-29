#pragma once
#include "object.h"
#include <GL/glew.h>
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <string>
#include <unordered_map>

enum class ShaderType {
	Custom = -1,
	BoundSDF,
	InfiniteSDF,
	VoxelData,
	ProceduralVoxels,
	SDFVolume,
	SDFSurface
};

class Shader : public Object {
private:
	GLuint program;
	std::unordered_map<std::string, GLint> nameToLocation;

	void AddNameIfNeeded(const std::string& name);

protected:
	bool ReadFile(const std::string& path, std::string& content);
	void CreateProgram(const std::string& vertexShaderContent, const std::string& fragmentShaderContent);

public:
	ShaderType type;

	Shader();
	Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	~Shader();

	void Use() const;
	void StopUsing() const;

	void SetInt(const std::string& name, GLint value);
	void SetFloat(const std::string& name, GLfloat value);
	void SetVec3(const std::string& name, const glm::vec3& value);
	void SetMat4(const std::string& name, const glm::mat4& value);
};