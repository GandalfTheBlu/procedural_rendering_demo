#include "transform.h"
#include <gtx/transform.hpp>

Transform::Transform() {
	this->worldPosition = glm::vec3(0.f);
	this->worldRotation = glm::identity<glm::fquat>();
	this->worldScale = glm::vec3(1.f);
	this->scale = glm::vec3(1.f, 1.f, 1.f);
	this->rotation = glm::identity<glm::fquat>();
	this->position = glm::vec3(0.f);
	this->matrix = glm::mat4(1.f);

	this->SetParent(TransformHandler::Instance().root);
}

Transform::Transform(bool freeStanding) {
	this->worldPosition = glm::vec3(0.f);
	this->worldRotation = glm::identity<glm::fquat>();
	this->worldScale = glm::vec3(1.f);
	this->scale = glm::vec3(1.f, 1.f, 1.f);
	this->rotation = glm::identity<glm::fquat>();
	this->position = glm::vec3(0.f);
	this->matrix = glm::mat4(1.f);

	if (!freeStanding) {
		this->SetParent(TransformHandler::Instance().root);
	}
	else {
		this->parent = nullptr;
	}
}

void Transform::Update() {
	this->matrix = (this->parent != nullptr ? this->parent->matrix : glm::mat4(1.f)) * 
		glm::translate(this->position) * glm::mat4_cast(this->rotation) * glm::scale(this->scale);

	if (this->parent != nullptr) {
		this->worldPosition = this->parent->worldPosition + this->position;
		this->worldRotation = this->parent->worldRotation * this->rotation;
		this->worldScale = this->parent->worldScale * this->scale;
	}
	else {
		this->worldPosition = this->position;
		this->worldRotation = this->rotation;
		this->worldScale = this->scale;
	}

	for (size_t i = 0; i < this->children.size(); i++) {
		this->children[i]->Update();
	}
}

void Transform::SetParent(Transform* parent) {
	this->parent = parent;
	if (parent != nullptr) {
		parent->children.push_back(this);
	}
}

void Transform::ChangeParent(Transform* newParent) {
	this->RemoveFromParent();
	this->SetParent(newParent);
}

void Transform::RemoveFromParent() {
	if (this->parent != nullptr) {
		for (size_t i = 0; i < this->parent->children.size(); i++) {
			if (this->parent->children[i] == this) {
				this->parent->children.erase(this->parent->children.begin() + i);
				this->parent = nullptr;
				break;
			}
		}
	}
}

glm::vec3 Transform::Right() const {
	return this->rotation * glm::vec3(1.f, 0.f, 0.f);
}

glm::vec3 Transform::Forward() const {
	return this->rotation * glm::vec3(0.f, 1.f, 0.f);
}

glm::vec3 Transform::Up() const {
	return this->rotation * glm::vec3(0.f, 0.f, 1.f);
}

glm::vec3 Transform::WorldPosition() const {
	return this->worldPosition;
}

glm::fquat Transform::WorldRotation() const {
	return this->worldRotation;
}

glm::vec3 Transform::WorldScale() const {
	return this->worldScale;
}


TransformHandler::TransformHandler() {
	this->root = new Transform(true);
}

TransformHandler::~TransformHandler() {
	delete this->root;
}

TransformHandler& TransformHandler::Instance() {
	static TransformHandler instance;
	return instance;
}