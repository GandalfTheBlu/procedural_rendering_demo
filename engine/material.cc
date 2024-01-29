#include "material.h"
#include "voxelData.h"

Material::Material() {
	this->shader = nullptr;
	this->ApplyModularShaderSettings = nullptr;
	this->ApplyCustomShaderSettings = nullptr;
	this->CleanUpAfterDraw = nullptr;
	this->renderType = RenderType::Deferred;
}

Material::Material(Shader* shader, void(*ApplyCustomShaderSettings)(GraphicsObject*, Shader*, Camera&)) {
	this->shader = shader;
	this->ApplyModularShaderSettings = nullptr;
	this->ApplyCustomShaderSettings = nullptr;
	this->CleanUpAfterDraw = nullptr;
	this->renderType = RenderType::Deferred;

	switch (this->shader->type) {
	case ShaderType::InfiniteSDF:
		this->ApplyModularShaderSettings = [](GraphicsObject* obj, Shader* shader, Camera& cam) {

			shader->SetFloat("u_aspect", cam.aspect);
			shader->SetFloat("u_tan_half_fovy", glm::tan(0.5f * cam.fovy));
			shader->SetFloat("u_near", cam.near);
			shader->SetVec3("u_up", cam.transform.Up());
			shader->SetVec3("u_forward", cam.transform.Forward());
			shader->SetVec3("u_camPos", cam.transform.WorldPosition());
			shader->SetMat4("u_VP", cam.VP);
		};
		break;

	case ShaderType::VoxelData:
		this->ApplyModularShaderSettings = [](GraphicsObject* obj, Shader* shader, Camera& cam) {
			VoxelData* voxels = dynamic_cast<VoxelData*>(obj);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, voxels->voxelTexture);

			shader->SetFloat("u_aspect", cam.aspect);
			shader->SetFloat("u_tan_half_fovy", glm::tan(0.5f * cam.fovy));
			shader->SetFloat("u_near", cam.near);
			shader->SetVec3("u_up", cam.transform.Up());
			shader->SetVec3("u_forward", cam.transform.Forward());
			shader->SetVec3("u_camPos", cam.transform.WorldPosition());
			shader->SetMat4("u_VP", cam.VP);
			shader->SetMat4("u_invM", glm::inverse(voxels->transform.matrix));
			shader->SetMat4("u_M", voxels->transform.matrix);
		};
		this->CleanUpAfterDraw = []() {
			glBindTexture(GL_TEXTURE_3D, 0);
		};
		break;

	case ShaderType::BoundSDF:
		this->ApplyModularShaderSettings = [](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetFloat("u_aspect", cam.aspect);
			shader->SetFloat("u_tan_half_fovy", glm::tan(0.5f * cam.fovy));
			shader->SetFloat("u_near", cam.near);
			shader->SetVec3("u_up", cam.transform.Up());
			shader->SetVec3("u_forward", cam.transform.Forward());
			shader->SetVec3("u_camPos", cam.transform.WorldPosition());
			shader->SetMat4("u_VP", cam.VP);
			shader->SetVec3("u_volumeSize", obj->transform.WorldScale());
			shader->SetVec3("u_volumePos", obj->transform.WorldPosition());
		};
		break;

	case ShaderType::ProceduralVoxels:
		this->ApplyModularShaderSettings = [](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetFloat("u_aspect", cam.aspect);
			shader->SetFloat("u_tan_half_fovy", glm::tan(0.5f * cam.fovy));
			shader->SetFloat("u_near", cam.near);
			shader->SetVec3("u_up", cam.transform.Up());
			shader->SetVec3("u_forward", cam.transform.Forward());
			shader->SetVec3("u_camPos", cam.transform.WorldPosition());
			shader->SetMat4("u_VP", cam.VP);
		};
		break;

	case ShaderType::SDFSurface:
		this->ApplyModularShaderSettings = [](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetMat4("u_M", obj->transform.matrix);
			shader->SetMat4("u_VP", cam.VP);
			shader->SetMat4("u_MVP", cam.VP * obj->transform.matrix);
			shader->SetVec3("u_camPos", cam.transform.WorldPosition());
		};
		break;

	case ShaderType::SDFVolume:
		this->ApplyModularShaderSettings = [](GraphicsObject* obj, Shader* shader, Camera& cam) {
			glm::vec3 camLocalPos = glm::vec3(glm::inverse(obj->transform.matrix) * glm::vec4(cam.transform.WorldPosition(), 1.f));

			shader->SetVec3("u_camLocalPos", camLocalPos);
			shader->SetMat4("u_M", obj->transform.matrix);
			shader->SetMat4("u_MVP", cam.VP * obj->transform.matrix);
			shader->SetMat4("u_VP", cam.VP);
		};
		break;

	default:
		this->ApplyModularShaderSettings = nullptr;
	}

	this->ApplyCustomShaderSettings = ApplyCustomShaderSettings;
}
Material::~Material() {}