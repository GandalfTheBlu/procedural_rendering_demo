#include "voxelData.h"

VoxelData::VoxelData(){
	this->voxelTexture = 0;
}
VoxelData::VoxelData(Mesh* mesh, const std::vector<Material*>& materials) : GraphicsObject(mesh, materials) {
	this->voxelTexture = 0;
}
VoxelData::~VoxelData(){
	glDeleteTextures(1, &this->voxelTexture);
}

void VoxelData::Generate(const glm::ivec3& resolution, float (*sdf)(const glm::vec3&)) {
	this->resolution = resolution;
	glm::ivec3 halfRes = resolution / 2;
	this->voxels.reserve((size_t)halfRes.x * halfRes.y * halfRes.z);

	for (size_t z = 0; z < halfRes.z; z++) {
		for (size_t y = 0; y < halfRes.y; y++) {
			for (size_t x = 0; x < halfRes.x; x++) {

				GLubyte num = 0;

				size_t i = 0;
				for (size_t subz = 0; subz < 2; subz++) {
					for (size_t suby = 0; suby < 2; suby++) {
						for (size_t subx = 0; subx < 2; subx++) {
							float value = sdf(glm::vec3(2 * x + subx, 2 * y + suby, 2 * z + subz));
							num |= (value < 0.f ? 1 : 0) << i++;
						}
					}
				}

				this->voxels.push_back(num);
			}
		}
	}
}

void VoxelData::UploadData() {
	
	glm::ivec3 halfRes = this->resolution / 2;

	glGenTextures(1, &this->voxelTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, this->voxelTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, halfRes.x, halfRes.y, halfRes.z, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, (const void*)(&this->voxels[0]));
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_3D, 0);
}