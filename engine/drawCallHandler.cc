#include "drawCallHandler.h"

RenderNode::RenderNode() {
	this->material = nullptr;
	this->subMeshIndex = 0;
	this->graphicsObject = nullptr;
}
RenderNode::RenderNode(Material* material, size_t subMeshIndex, GraphicsObject* graphicsObject) {
	this->material = material;
	this->subMeshIndex = subMeshIndex;
	this->graphicsObject = graphicsObject;
}
RenderNode::~RenderNode() {}


DrawCallHandler::DrawCallHandler(){}
DrawCallHandler::~DrawCallHandler(){}

void DrawCallHandler::Add(GraphicsObject* graphicsObject) {
	for (size_t i = 0; i < graphicsObject->materials.size(); i++) {
		Material* material = graphicsObject->materials[i];
		Shader* shader = material->shader;
		this->shaderIdToRenderNode[shader].push_back(RenderNode(material, i, graphicsObject));
	}
}

void DrawCallHandler::Remove(GraphicsObject* graphicsObject) {
	for (size_t i = 0; i < graphicsObject->materials.size(); i++) {
		
		Material* material = graphicsObject->materials[i];
		Shader* shader = material->shader;
		if (this->shaderIdToRenderNode.count(shader) != 0) {
			
			std::vector<RenderNode>& renderNodes = this->shaderIdToRenderNode[shader];
			for (size_t j = 0; j < renderNodes.size(); j++) {
				if (renderNodes[j].graphicsObject == graphicsObject) {
					renderNodes.erase(renderNodes.begin() + j);
					break;
				}
			}
		}
	}
}

void DrawCallHandler::Draw(Camera& camera) {
	for (auto elem : this->shaderIdToRenderNode) {
		
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