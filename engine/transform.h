#pragma once
#include <gtx/quaternion.hpp>
#include <vector>

class Transform {
private:
	void SetParent(Transform* parent);
	void RemoveFromParent();

	glm::vec3 worldPosition;
	glm::fquat worldRotation;
	glm::vec3 worldScale;

public:
	glm::vec3 scale;
	glm::fquat rotation;
	glm::vec3 position;
	glm::mat4x4 matrix;

	Transform* parent;
	std::vector<Transform*> children;

	Transform();
	Transform(bool freeStanding);

	void Update();
	void ChangeParent(Transform* newParent);
	glm::vec3 Right() const;
	glm::vec3 Forward() const;
	glm::vec3 Up() const;
	glm::vec3 WorldPosition() const;
	glm::fquat WorldRotation() const;
	glm::vec3 WorldScale() const;
};

class TransformHandler {
private:
	TransformHandler();

public:
	~TransformHandler();

	static TransformHandler& Instance();

	Transform* root;
};