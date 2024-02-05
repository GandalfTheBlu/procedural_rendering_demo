#include "deferred.h"
#include <gtx/transform.hpp>
#include <iostream>

glm::fquat Math::AlignToDirection(const glm::vec3& direction) {
	glm::vec3 up = direction.z != 1.f ? glm::vec3(0.f, 0.f, 1.f) : glm::vec3(1.f, 0.f, 0.f);
	up = glm::normalize(glm::cross(glm::cross(direction, up), direction));
	glm::vec3 right = glm::cross(direction, up);
	glm::mat3 m(
		right,
		direction,
		up
	);

	return glm::quat_cast(m);
}


Renderer::RenderNode::RenderNode() {
	this->material = nullptr;
	this->subMeshIndex = 0;
	this->graphicsObject = nullptr;
}
Renderer::RenderNode::RenderNode(Material* material, size_t subMeshIndex, GraphicsObject* graphicsObject) {
	this->material = material;
	this->subMeshIndex = subMeshIndex;
	this->graphicsObject = graphicsObject;
}
Renderer::RenderNode::~RenderNode() {}


float PointLight::minIntensity = 0.004f;

PointLight::PointLight() {
	this->color = glm::vec3(1.f, 1.f, 1.f);
	this->attenuation = glm::vec3(1.8f, 0.7f, 1.f);
	this->visible = true;
	CalcRadius();
}
PointLight::PointLight(const glm::vec3& position, const glm::vec3& color, const glm::vec3& attenuation) {
	this->transform.position = position;
	this->color = color;
	this->attenuation = attenuation;
	this->visible = true;
	CalcRadius();
}

void PointLight::CalcRadius() {
	float q = this->attenuation.x;
	float l = this->attenuation.y;
	float c = this->attenuation.z;
	float min_inv = 1.f / PointLight::minIntensity;
	float max = glm::max(this->color.x, glm::max(this->color.y, this->color.z));

	this->radius = (-l + glm::sqrt(l * l - 4.f * q * (c - max * min_inv))) / (2.f * q);
}

float SpotLight::minIntensity = 0.004f;

SpotLight::SpotLight() {
	this->color = glm::vec3(1.f, 1.f, 1.f);
	this->attenuation = glm::vec3(1.8f, 0.7f, 1.f);
	this->angle = 1.5f;
	this->visible = true;
	CalcRadius();
}
SpotLight::SpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, const glm::vec3 attenuation, float angle) {
	this->transform.position = position;
	this->transform.rotation = Math::AlignToDirection(direction);
	this->color = color;
	this->attenuation = attenuation;
	this->angle = angle;
	this->visible = true;
	CalcRadius();
}

void SpotLight::CalcRadius() {
	float q = this->attenuation.x;
	float l = this->attenuation.y;
	float c = this->attenuation.z;
	float min_inv = 1.f / SpotLight::minIntensity;
	float max = glm::max(this->color.x, glm::max(this->color.y, this->color.z));

	this->radius = (-l + glm::sqrt(l * l - 4.f * q * (c - max * min_inv))) / (2.f * q);
}


DirectionalLight::DirectionalLight() {
	this->direction = glm::vec3(0.f, -1.f, 0.f);
	this->lightColor = glm::vec3(1.f, 1.f, 1.f);
	this->intensity = 1.f;
}
DirectionalLight::DirectionalLight(const glm::vec3& direction, const glm::vec3& lightColor, const glm::vec3& ambientColor, float intensity) {
	this->direction = direction;
	this->lightColor = lightColor;
	this->ambientColor = ambientColor;
	this->intensity = intensity;
}


Renderer::Renderer() {
	this->geometryFramebuffer = 0;
	this->shadingFramebuffer = 0;
	this->positionBuffer = 0;
	this->normalBuffer = 0;
	this->albedoSpecBuffer = 0;
	this->depthStencilBuffer = 0;
	this->finalColorBuffer = 0;
	this->postProcessingBuffer = 0;
	this->postProcessingFramebuffer = 0;

	this->pointLightMeshShader = nullptr;
	this->spotLightMeshShader = nullptr;
	this->directionalLightShader = nullptr;
	this->finalColorShader = nullptr;
	this->debugFlatShader = nullptr;
	this->screenQuad = nullptr;
	this->pointLightMesh = nullptr;
	this->spotLightMesh = nullptr;
	this->directionalLight = nullptr;
	this->skyBoxMaterial = nullptr;

	this->debugLightSources = false;
	this->debugLightMeshes = false;
}
Renderer::~Renderer() {
	glDeleteFramebuffers(1, &this->geometryFramebuffer);
	glDeleteFramebuffers(1, &this->shadingFramebuffer);
	glDeleteFramebuffers(1, &this->postProcessingFramebuffer);
	glDeleteTextures(1, &this->positionBuffer);
	glDeleteTextures(1, &this->normalBuffer);
	glDeleteTextures(1, &this->albedoSpecBuffer);
	glDeleteTextures(1, &this->depthStencilBuffer);
	glDeleteTextures(1, &this->finalColorBuffer);
	glDeleteTextures(1, &this->postProcessingBuffer);
}

Renderer& Renderer::Instance() {
	static Renderer renderer;
	return renderer;
}

std::unordered_map<Shader*, std::vector<Renderer::RenderNode>>& Renderer::GetRenderNodeContainer(Material::RenderType renderType) {
	switch (renderType) {
	case Material::RenderType::Deferred:
		return this->shaderIdToRenderNode;
	case Material::RenderType::Forward:
		return this->shaderIdToForwardRenderNode;
	}

	return this->shaderIdToUIRenderNode;
}

void Renderer::Add(GraphicsObject* graphicsObject) {
	for (size_t i = 0; i < graphicsObject->materials.size(); i++) {
		Material* material = graphicsObject->materials[i];
		Shader* shader = material->shader;

		this->GetRenderNodeContainer(material->renderType)[shader].push_back(RenderNode(material, i, graphicsObject));
	}
}

void Renderer::Remove(GraphicsObject* graphicsObject) {
	for (size_t i = 0; i < graphicsObject->materials.size(); i++) {

		Material* material = graphicsObject->materials[i];
		Shader* shader = material->shader;
		std::unordered_map<Shader*, std::vector<RenderNode>>& renderNodeContainer = this->GetRenderNodeContainer(material->renderType);

		if (renderNodeContainer.count(shader) != 0) {

			std::vector<RenderNode>& renderNodes = renderNodeContainer[shader];
			for (size_t j = 0; j < renderNodes.size(); j++) {
				if (renderNodes[j].graphicsObject == graphicsObject) {
					renderNodes.erase(renderNodes.begin() + j);
					break;
				}
			}
		}
	}
}

void Renderer::AddPostProcessingMaterial(Material* postProcessingMaterial) {
	if (this->postProcessingBuffer == 0) {
		this->CreatePostProcessingFramebuffer();
	}

	this->postProcessingMaterials.push_back(postProcessingMaterial);
}

void Renderer::RemovePostProcessingMaterial(Material* postProcessingMaterial) {
	for (size_t i = 0; i < this->postProcessingMaterials.size(); i++) {
		if (this->postProcessingMaterials[i] == postProcessingMaterial) {
			this->postProcessingMaterials.erase(this->postProcessingMaterials.begin() + i);
			return;
		}
	}
}

void Renderer::BindGBuffer(Shader* shader) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->positionBuffer);
	shader->SetInt("g_worldPos", 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, this->normalBuffer);
	shader->SetInt("g_normal", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, this->albedoSpecBuffer);
	shader->SetInt("g_albedoSpec", 2);
}

void Renderer::UnbindGBuffer() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::BindGPosition(GLuint binding) {
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, this->positionBuffer);
}

void Renderer::BindGNormal(GLuint binding) {
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, this->normalBuffer);
}

void Renderer::BindGAlbedoSpec(GLuint binding) {
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, this->albedoSpecBuffer);
}

void Renderer::UnbindGTexture(GLuint binding) {
	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::Draw(Camera& camera) {

	if (this->shaderIdToRenderNode.size() > 0) {
		this->DrawGeometry(camera);
		this->CullLights(camera);
		this->DrawLightMeshes(camera);
		this->DrawDirectionalLight(camera);
	}

	if (this->skyBoxMaterial != nullptr) {
		this->DrawSkyBox(camera);
	}
	
	if (this->shaderIdToForwardRenderNode.size() > 0) {
		this->DrawForwardRendering(camera);
	}

	if (this->postProcessingMaterials.size() > 0) {
		this->DrawPostProcessing(camera);
	}

	if (this->debugLightSources) {
		this->DrawDebugLightMeshes(camera, GL_FILL, 0.1f, false);
	}
	if (this->debugLightMeshes) {
		this->DrawDebugLightMeshes(camera, GL_LINE, 1.f, true);
	}

	if (this->shaderIdToUIRenderNode.size() > 0) {
		this->DrawUI(camera);
	}

	this->DrawToMainFramebuffer();
}

void Renderer::CreatePostProcessingFramebuffer() {
	// create a post processing framebuffer
	glGenFramebuffers(1, &this->postProcessingFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->postProcessingFramebuffer);

	// add an output texture
	glGenTextures(1, &this->postProcessingBuffer);
	glBindTexture(GL_TEXTURE_2D, this->postProcessingBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->screenSize.x, this->screenSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->postProcessingBuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Renderer: post processing buffer not completed" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::Setup(int screenWidth, int screenHeight) {
	this->screenSize = glm::uvec2((GLuint)screenWidth, (GLuint)screenHeight);

	// create geometry framebuffer and attach textures (position, normal, albedo/specular)
	glGenFramebuffers(1, &this->geometryFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->geometryFramebuffer);

	glGenTextures(1, &this->positionBuffer);
	glBindTexture(GL_TEXTURE_2D, this->positionBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->positionBuffer, 0);

	glGenTextures(1, &this->normalBuffer);
	glBindTexture(GL_TEXTURE_2D, this->normalBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, this->normalBuffer, 0);

	glGenTextures(1, &this->albedoSpecBuffer);
	glBindTexture(GL_TEXTURE_2D, this->albedoSpecBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, this->albedoSpecBuffer, 0);

	GLuint attachments[]{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// create and add a depth- and stencil buffer
	glGenRenderbuffers(1, &this->depthStencilBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, this->depthStencilBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->depthStencilBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Renderer: geometry buffer not completed" << std::endl;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearStencil(0);

	// create a buffer for the shading steps where the lights will be drawn
	glGenFramebuffers(1, &this->shadingFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, this->shadingFramebuffer);

	// add an output texture
	glGenTextures(1, &this->finalColorBuffer);
	glBindTexture(GL_TEXTURE_2D, this->finalColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->finalColorBuffer, 0);

	// the shading framebuffer shares the depth and stencil buffer with the geometry framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, this->depthStencilBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Renderer: shading buffer not completed" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);


	this->screenQuad = new Mesh();
	Mesh::ScreenQuad(*this->screenQuad);

	this->pointLightMesh = new Mesh();
	Mesh::LightSphere(1.f, 10, 10, *pointLightMesh);

	this->spotLightMesh = new Mesh();
	Mesh::LightCone(1.f, 1.f, 10, *spotLightMesh);

	this->directionalLight = new DirectionalLight();

	// load shaders for the different stages
	this->pointLightMeshShader = new Shader(
		"shaders/deferred/deferred_light_vertex.glsl",
		"shaders/deferred/deferred_point_light_fragment.glsl"
	);
	this->spotLightMeshShader = new Shader(
		"shaders/deferred/deferred_light_vertex.glsl",
		"shaders/deferred/deferred_spot_light_fragment.glsl"
	);
	this->directionalLightShader = new Shader(
		"shaders/deferred/full_screen_vertex.glsl",
		"shaders/deferred/deferred_directional_fragment.glsl"
	);
	this->finalColorShader = new Shader(
		"shaders/deferred/full_screen_vertex.glsl",
		"shaders/deferred/full_screen_texture_fragment.glsl"
	);
	this->debugFlatShader = new Shader(
		"shaders/deferred/flat_vertex.glsl",
		"shaders/deferred/flat_fragment.glsl"
	);
}

void Renderer::DrawSorted(std::unordered_map<Shader*, std::vector<RenderNode>>& renderNodeContainer, Camera& camera) {
	for (auto elem : renderNodeContainer) {

		Shader* shader = elem.first;
		shader->Use();

		for (size_t i = 0; i < elem.second.size(); i++) {

			RenderNode& renderNode = elem.second[i];

			if (renderNode.material->ApplyModularShaderSettings != nullptr) {
				renderNode.material->ApplyModularShaderSettings(renderNode.graphicsObject, shader, camera);
			}
			if (renderNode.material->ApplyCustomShaderSettings != nullptr) {
				renderNode.material->ApplyCustomShaderSettings(renderNode.graphicsObject, shader, camera);
			}

			Mesh* mesh = renderNode.graphicsObject->mesh;

			mesh->Bind();
			mesh->Draw(renderNode.subMeshIndex);
			mesh->Unbind();

			if (renderNode.material->CleanUpAfterDraw != nullptr) {
				renderNode.material->CleanUpAfterDraw();
			}
		}

		shader->StopUsing();
	}
}

void Renderer::DrawGeometry(Camera& camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, this->geometryFramebuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);// enable write to depth buffer
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDisable(GL_STENCIL_TEST);

	this->DrawSorted(this->shaderIdToRenderNode, camera);
}

void Renderer::CullLights(Camera& camera) {

	glm::mat4 toCamSpace = glm::inverse(camera.transform.matrix);

	for (int i = 0; i < this->pointLights.size(); i++) {
		glm::vec3 p = this->pointLights[i]->transform.WorldPosition();
		glm::vec3 camSpaceP = glm::vec3(toCamSpace * glm::vec4(p, 1.f));

		float r = this->pointLights[i]->radius;

		this->pointLights[i]->visible = true;

		for (int j = 0; j < 6; j++) {
			Camera::FrustumPlane* plane = camera.frustum[j];
			// positive if inside frustum
			float signedDistance = glm::dot(camSpaceP, plane->normal) - plane->offset;
			
			// sphere is outside frustum
			if (signedDistance + r < 0.f) {
				this->pointLights[i]->visible = false;
				break;
			}
		}
	}

	// the cone culling is not exact but it is a decent approximation
	// it has some false positives, but not the other way around
	for (int i = 0; i < this->spotLights.size(); i++) {
		SpotLight* sl = this->spotLights[i];
		glm::vec3 p = sl->transform.WorldPosition();
		glm::vec3 d = glm::mat3_cast(sl->transform.WorldRotation())[1];

		glm::vec3 tip = glm::vec3(toCamSpace * glm::vec4(p.x, p.y, p.z, 1.f));
		glm::vec3 direction = glm::vec3(toCamSpace * glm::vec4(d.x, d.y, d.z, 0.f));

		float baseRadius = sl->radius * glm::tan(sl->angle * 0.5f);

		sl->visible = true;

		for (int j = 0; j < 6; j++) {
			Camera::FrustumPlane* plane = camera.frustum[j];

			float tipPlaneOffset = glm::dot(tip, plane->normal) - plane->offset;
			// if tip is on the right side of the plane, then the cone is visible for this plane
			if (tipPlaneOffset >= 0.f) {
				continue;
			}

			float dirNormCompare = glm::dot(direction, plane->normal);
			// two special cases for when the direction is colinear to the normal
			if (dirNormCompare == -1.f) {
				sl->visible = false;
				break;
			}
			else if (dirNormCompare == 1.f) {
				// simply check if the cone can reach the plane along the direction vector
				if (-tipPlaneOffset > sl->radius) {
					sl->visible = false;
					break;
				}
			}
			else {
				glm::vec3 baseCenter = tip + direction * sl->radius;
				// a vector from the center of the base out to the edge, coplanar with the plane created by the direction and the normal
				glm::vec3 coplanarRadius = glm::normalize(glm::cross(glm::cross(direction, plane->normal), direction)) * baseRadius;
				glm::vec3 edgePoint = baseCenter + coplanarRadius;

				// check if the edge point is on the right side of the plane
				if (glm::dot(edgePoint, plane->normal) - plane->offset < 0.f)
				{
					// test the other edge
					edgePoint = baseCenter - coplanarRadius;

					if (glm::dot(edgePoint, plane->normal) - plane->offset < 0.f) {
						sl->visible = false;
						break;
					}
				}

				// some false positives manage to get here since the cone doesnt have to intersect the frustum
				// for the edge points to be on the right side of the plane
			}
		}
	}
}

void Renderer::DrawLightMeshes(Camera& camera) {
	glBindFramebuffer(GL_FRAMEBUFFER, this->shadingFramebuffer);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);// disable write to depth buffer

	glEnable(GL_STENCIL_TEST);
	glStencilMask(0xFF);// enable write to stencil buffer

	// additive lighting
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	// --- point lights ---
	if (this->pointLights.size() > 0) {
		// provide shader with access to the geometry buffers
		this->pointLightMeshShader->Use();
		this->BindGBuffer(this->pointLightMeshShader);

		this->pointLightMeshShader->SetVec3("u_camWorldPos", camera.transform.WorldPosition());
		const glm::mat4& VP = camera.VP;

		// draw point lights
		this->pointLightMesh->Bind();
		for (int i = 0; i < this->pointLights.size(); i++) {
			if (!this->pointLights[i]->visible) {
				continue;
			}

			float r = this->pointLights[i]->radius;
			glm::mat4 MVP = VP * this->pointLights[i]->transform.matrix * glm::scale(glm::vec3(r));
			this->pointLightMeshShader->SetMat4("u_MVP", MVP);
			this->pointLightMeshShader->SetVec3("u_lightWorldPos", this->pointLights[i]->transform.WorldPosition());
			this->pointLightMeshShader->SetVec3("u_lightColor", this->pointLights[i]->color);
			this->pointLightMeshShader->SetVec3("u_lightAttenuation", this->pointLights[i]->attenuation);
			this->pointLightMeshShader->SetFloat("u_lightRadius", this->pointLights[i]->radius);

			// first pass for stencil
			this->pointLightMeshShader->SetInt("u_stage", 0);

			glStencilFunc(GL_ALWAYS, 0, 0xFF);// always pass the stencil test
			glStencilOp(GL_ZERO, GL_INCR, GL_ZERO);// set to 1 if depth fail, set to 0 if depth pass

			// render front face
			glCullFace(GL_BACK);
			glDepthFunc(GL_LESS);

			this->pointLightMesh->Draw(0);

			// second pass for shading
			this->pointLightMeshShader->SetInt("u_stage", 1);

			glStencilFunc(GL_EQUAL, 0, 0xFF);// pass the stencil if stencil is 0
			glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);// reset stencil to 0 if it was 1

			// render back face
			glCullFace(GL_FRONT);
			glDepthFunc(GL_GREATER);

			this->pointLightMesh->Draw(0);
		}
		this->pointLightMesh->Unbind();
		this->UnbindGBuffer();
		this->pointLightMeshShader->StopUsing();
	}


	// --- spot lights ---
	if (this->spotLights.size() > 0) {
		// provide shader with access to the geometry buffers
		this->spotLightMeshShader->Use();
		this->BindGBuffer(this->spotLightMeshShader);

		this->spotLightMeshShader->SetVec3("u_camWorldPos", camera.transform.WorldPosition());
		const glm::mat4& VP = camera.VP;

		// draw spot lights
		this->spotLightMesh->Bind();
		for (int i = 0; i < this->spotLights.size(); i++) {
			if (!this->spotLights[i]->visible) {
				continue;
			}

			float r = this->spotLights[i]->radius;
			float w = r * glm::tan(this->spotLights[i]->angle * 0.5f);
			glm::mat4 rot = glm::mat4_cast(this->spotLights[i]->transform.WorldRotation());
			glm::mat4 MVP = VP * this->spotLights[i]->transform.matrix * glm::scale(glm::vec3(w, w, r));


			this->spotLightMeshShader->SetMat4("u_MVP", MVP);
			this->spotLightMeshShader->SetVec3("u_lightWorldPos", this->spotLights[i]->transform.WorldPosition());
			this->spotLightMeshShader->SetVec3("u_lightWorldDirection", rot[1]);
			this->spotLightMeshShader->SetVec3("u_lightColor", this->spotLights[i]->color);
			this->spotLightMeshShader->SetVec3("u_lightAttenuation", this->spotLights[i]->attenuation);
			this->spotLightMeshShader->SetFloat("u_lightRadius", this->spotLights[i]->radius);
			this->spotLightMeshShader->SetFloat("u_lightAngle", this->spotLights[i]->angle);

			// first pass for stencil
			this->spotLightMeshShader->SetInt("u_stage", 0);

			glStencilFunc(GL_ALWAYS, 0, 0xFF);// always pass the stencil test
			glStencilOp(GL_ZERO, GL_INCR, GL_ZERO);// set to 1 if depth fail, set to 0 if depth pass

			// render front face
			glCullFace(GL_BACK);
			glDepthFunc(GL_LESS);

			this->spotLightMesh->Draw(0);

			// second pass for shading
			this->spotLightMeshShader->SetInt("u_stage", 1);

			glStencilFunc(GL_EQUAL, 0, 0xFF);// pass the stencil if stencil is 0
			glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);// reset stencil to 0 if it was 1

			// render back face
			glCullFace(GL_FRONT);
			glDepthFunc(GL_GREATER);

			this->spotLightMesh->Draw(0);
		}
		this->spotLightMesh->Unbind();
		this->UnbindGBuffer();
		this->spotLightMeshShader->StopUsing();
	}


	// clean up
	glDisable(GL_STENCIL_TEST);
	glDepthFunc(GL_LESS);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);// enable write to depth
}

void Renderer::DrawDirectionalLight(Camera& camera) {
	glCullFace(GL_BACK);

	glDepthMask(GL_FALSE);// disable write to depth buffer
	glDisable(GL_DEPTH_TEST);

	// additive lighting
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	this->directionalLightShader->Use();

	this->directionalLightShader->SetVec3("u_camWorldPos", camera.transform.WorldPosition());
	this->directionalLightShader->SetVec3("u_ambientColor", this->directionalLight->ambientColor);
	this->directionalLightShader->SetVec3("u_lightDirection", this->directionalLight->direction);
	this->directionalLightShader->SetVec3("u_lightColor", this->directionalLight->lightColor);
	this->directionalLightShader->SetFloat("u_intensity", this->directionalLight->intensity);

	// provide shader with access to the geometry buffers
	this->BindGBuffer(this->directionalLightShader);

	this->screenQuad->Bind();
	this->screenQuad->Draw(0);
	this->screenQuad->Unbind();
	this->directionalLightShader->StopUsing();

	this->UnbindGBuffer();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);// enable write to depth
}

void Renderer::DrawSkyBox(Camera& camera) {
	this->screenQuad->Bind();
	glDepthMask(GL_FALSE);

	this->skyBoxMaterial->shader->Use();

	if (this->skyBoxMaterial->ApplyCustomShaderSettings != nullptr) {
		this->skyBoxMaterial->ApplyCustomShaderSettings(nullptr, this->skyBoxMaterial->shader, camera);
	}

	this->screenQuad->Draw(0);

	if (this->skyBoxMaterial->CleanUpAfterDraw != nullptr) {
		this->skyBoxMaterial->CleanUpAfterDraw();
	}

	glDepthMask(GL_TRUE);
	this->screenQuad->Unbind();
}

void Renderer::DrawForwardRendering(Camera& camera) {
	this->DrawSorted(this->shaderIdToForwardRenderNode, camera);
}

void Renderer::DrawDebugLightMeshes(Camera& camera, GLenum polygonMode, float scale, bool scaleWithRadius) {
	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
	glDisable(GL_CULL_FACE);

	this->debugFlatShader->Use();
	glm::mat4& VP = camera.VP;

	if (this->pointLights.size() > 0) {
		this->pointLightMesh->Bind();
		for (int i = 0; i < this->pointLights.size(); i++) {
			if (!this->pointLights[i]->visible) {
				continue;
			}

			float r = scale * (scaleWithRadius ? this->pointLights[i]->radius : 1.f);
			glm::mat4 MVP = VP * this->pointLights[i]->transform.matrix * glm::scale(glm::vec3(r));
			this->debugFlatShader->SetMat4("u_MVP", MVP);
			this->debugFlatShader->SetVec3("u_color", this->pointLights[i]->color);
			this->pointLightMesh->Draw(0);
		}
		this->pointLightMesh->Unbind();
	}

	if (this->spotLights.size() > 0) {
		this->spotLightMesh->Bind();
		for (int i = 0; i < this->spotLights.size(); i++) {
			if (!this->spotLights[i]->visible) {
				continue;
			}
			float r = scale * (scaleWithRadius ? this->spotLights[i]->radius : 1.f);
			float w = r * tanf(this->spotLights[i]->angle * 0.5f);
			glm::mat4 MVP = VP * this->spotLights[i]->transform.matrix * glm::scale(glm::vec3(w, r, w));

			this->debugFlatShader->SetMat4("u_MVP", MVP);
			this->debugFlatShader->SetVec3("u_color", this->spotLights[i]->color);
			this->spotLightMesh->Draw(0);
		}
		this->spotLightMesh->Unbind();
	}

	this->debugFlatShader->StopUsing();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Renderer::DrawUI(Camera& camera){
	this->DrawSorted(this->shaderIdToUIRenderNode, camera);
}

void Renderer::DrawToMainFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// draw the final color texture to the screen
	this->finalColorShader->Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->finalColorBuffer);
	this->finalColorShader->SetInt("u_texture", 0);

	this->screenQuad->Bind();
	this->screenQuad->Draw(0);
	this->screenQuad->Unbind();

	glBindTexture(GL_TEXTURE_2D, 0);
	this->finalColorShader->StopUsing();
}

void Renderer::DrawPostProcessing(Camera& camera) {
	this->screenQuad->Bind();

	for (size_t i = 0; i < this->postProcessingMaterials.size(); i++) {

		// switch between framebuffers
		// they will take turns reading from each others target textures
		if (i % 2 == 0) {
			glBindFramebuffer(GL_FRAMEBUFFER, this->postProcessingFramebuffer);
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, this->shadingFramebuffer);
		}
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		Material* material = this->postProcessingMaterials[i];
		material->shader->Use();

		// bind the other framebuffer's target texture to write to it
		// also remember which texture contains the final result for when drawing to main framebuffer
		if (i % 2 == 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->finalColorBuffer);
			material->shader->SetInt("t_color", 0);
		}
		else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->postProcessingBuffer);
			material->shader->SetInt("t_color", 0);
		}


		if (material->ApplyCustomShaderSettings != nullptr) {
			material->ApplyCustomShaderSettings(nullptr, material->shader, camera);
		}

		this->screenQuad->Draw(0);

		if (material->CleanUpAfterDraw != nullptr) {
			material->CleanUpAfterDraw();
		}

		material->shader->StopUsing();

		glBindTexture(GL_TEXTURE_2D, 0);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

	// if the last write was to the post processing texture, then copy that to the final color texture
	// so that later stages can use that and the shading frambuffer's depth buffer
	if (this->postProcessingMaterials.size() % 2 == 1) {
		glBindFramebuffer(GL_FRAMEBUFFER, this->shadingFramebuffer);

		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		// write result to final color buffer
		this->finalColorShader->Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->postProcessingBuffer);
		this->finalColorShader->SetInt("u_texture", 0);

		this->screenQuad->Draw(0);

		glBindTexture(GL_TEXTURE_2D, 0);
		this->finalColorShader->StopUsing();

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
	}

	this->screenQuad->Unbind();
}