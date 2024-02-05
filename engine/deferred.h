#pragma once
#include "material.h"
#include "graphicsObject.h"
#include <unordered_map>

namespace Math {
	glm::fquat AlignToDirection(const glm::vec3& direction);
}

class PointLight : public Data {
public:
	Transform transform;
	glm::vec3 color;
	glm::vec3 attenuation;
	float radius;
	bool visible;

	static float minIntensity;

	PointLight();
	PointLight(const glm::vec3& position, const glm::vec3& color, const glm::vec3& attenuation);

	void CalcRadius();
};

class SpotLight : public Data {
public:
	Transform transform;
	glm::vec3 color;
	glm::vec3 attenuation;
	float angle;
	float radius;
	bool visible;

	static float minIntensity;

	SpotLight();
	SpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, const glm::vec3 attenuation, float angle);

	void CalcRadius();
};

class DirectionalLight : public Data {
public:
	glm::vec3 direction;
	glm::vec3 lightColor;
	glm::vec3 ambientColor;
	float intensity;

	DirectionalLight();
	DirectionalLight(const glm::vec3& direction, const glm::vec3& lightColor, const glm::vec3& ambientColor, float intensity);
};



class Renderer {
private:
	struct RenderNode {
	public:
		Material* material;
		size_t subMeshIndex;
		GraphicsObject* graphicsObject;

		RenderNode();
		RenderNode(Material* material, size_t subMeshIndex, GraphicsObject* graphicsObject);
		~RenderNode();
	};

	Shader* pointLightMeshShader;
	Shader* spotLightMeshShader;
	Shader* directionalLightShader;
	Shader* finalColorShader;
	Shader* debugFlatShader;
	Mesh* screenQuad;

	GLuint geometryFramebuffer;
	GLuint shadingFramebuffer;
	GLuint positionBuffer;
	GLuint normalBuffer;
	GLuint albedoSpecBuffer;
	GLuint depthStencilBuffer;
	GLuint finalColorBuffer;
	GLuint postProcessingFramebuffer;
	GLuint postProcessingBuffer;

	glm::uvec2 screenSize;

	std::unordered_map<Shader*, std::vector<RenderNode>> shaderIdToRenderNode;
	std::unordered_map<Shader*, std::vector<RenderNode>> shaderIdToForwardRenderNode;
	std::unordered_map<Shader*, std::vector<RenderNode>> shaderIdToUIRenderNode;
	std::vector<Material*> postProcessingMaterials;

	void CreatePostProcessingFramebuffer();
	void BindGBuffer(Shader* shader);
	void UnbindGBuffer();

	std::unordered_map<Shader*, std::vector<RenderNode>>& GetRenderNodeContainer(Material::RenderType renderType);
	void DrawSorted(std::unordered_map<Shader*, std::vector<RenderNode>>& renderNodeContainer, Camera& camera);

	void DrawGeometry(Camera& camera);
	void CullLights(Camera& camera);
	void DrawLightMeshes(Camera& camera);
	void DrawDirectionalLight(Camera& camera);
	void DrawSkyBox(Camera& camera);
	void DrawForwardRendering(Camera& camera);
	void DrawPostProcessing(Camera& camera);
	void DrawDebugLightMeshes(Camera& camera, GLenum polygonMode, float scale, bool scaleWithRadius = true);
	void DrawUI(Camera& camera);
	void DrawToMainFramebuffer();

	Renderer();

public:
	std::vector<PointLight*> pointLights;
	Mesh* pointLightMesh;
	std::vector<SpotLight*> spotLights;
	Mesh* spotLightMesh;
	DirectionalLight* directionalLight;

	Material* skyBoxMaterial;

	bool debugLightSources;
	bool debugLightMeshes;

	~Renderer();

	static Renderer& Instance();

	void Setup(int screenWidth, int screenHeight);
	void Add(GraphicsObject* graphicsObject);
	void Remove(GraphicsObject* graphicsObject);
	void AddPostProcessingMaterial(Material* postProcessingMaterial);
	void RemovePostProcessingMaterial(Material* postProcessingMaterial);
	void Draw(Camera& camera);

	void BindGPosition(GLuint binding);
	void BindGNormal(GLuint binding);
	void BindGAlbedoSpec(GLuint binding);

	void UnbindGTexture(GLuint binding);
};
