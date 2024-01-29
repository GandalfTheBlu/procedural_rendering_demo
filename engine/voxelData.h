#pragma once
#include <vector>
#include <GL/glew.h>
#include "graphicsObject.h"

class VoxelData : public GraphicsObject {
public:
	std::vector<GLubyte> voxels;
	GLuint voxelTexture;
	glm::ivec3 resolution;

	VoxelData();
	VoxelData(Mesh* mesh, const std::vector<Material*>& materials);
	~VoxelData();

	void Generate(const glm::ivec3& resolution, float (*sdf)(const glm::vec3&));
	void UploadData();
};