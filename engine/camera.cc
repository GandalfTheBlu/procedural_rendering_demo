#include "camera.h"
#include <gtc/matrix_transform.hpp>
#include <iostream>

Camera::FrustumPlane::FrustumPlane() {
	this->offset = 0.f;
	this->normal = glm::vec3(0.f);
}
Camera::FrustumPlane::FrustumPlane(const glm::vec3& normal, float offset) {
	this->normal = normal;
	this->offset = offset;
}


Camera::Frustum::Frustum() {}

Camera::FrustumPlane* Camera::Frustum::operator[](const int i) {
	switch (i) {
	case 0: return &this->left;
	case 1: return &this->right;
	case 2: return &this->bottom;
	case 3: return &this->top;
	case 4: return &this->near;
	case 5: return &this->far;
	}

	std::cout << "FrustumPlane: plane index out of range, must be [0, 5]" << std::endl;
	return nullptr;
}

Camera::Camera() {
	this->fovy = 0.f;
	this->aspect = 0.f;
	this->near = 0.f;
	this->far = 0.f;
	this->P = glm::mat4(0.f);
	this->V = glm::mat4(0.f);
	this->VP = glm::mat4(0.f);
}

Camera::Camera(float fovy, float aspect, float near, float far) {
	this->fovy = fovy;
	this->aspect = aspect;
	this->near = near;
	this->far = far;
	this->P = glm::mat4(0.f);
	this->V = glm::mat4(0.f);
	this->VP = glm::mat4(0.f);
	this->CalcFrustum();
}

Camera::~Camera() {}

void Camera::CalcFrustum() {
	float tan_half_fovy = glm::tan(this->fovy * 0.5f);

	// normals pointing into the frustum

	glm::vec3 topNormal = glm::normalize(glm::vec3(0.f, 1.f, -tan_half_fovy));
	this->frustum.top.normal = topNormal;
	this->frustum.bottom.normal = glm::vec3(0.f, topNormal.y, -topNormal.z);

	glm::vec3 rightNormal = glm::normalize(glm::vec3(-1.f, tan_half_fovy * this->aspect, 0.f));
	this->frustum.right.normal = rightNormal;
	this->frustum.left.normal = glm::vec3(-rightNormal.x, rightNormal.y, 0.f);

	this->frustum.near.normal = glm::vec3(0.f, 1.f, 0.f);
	this->frustum.near.offset = this->near;

	this->frustum.far.normal = glm::vec3(0.f, -1.f, 0.f);
	this->frustum.far.offset = -this->far;
}

void Camera::CalcMatrices() {
	glm::vec3 pos = this->transform.position;
	glm::vec3 forward = this->transform.Forward();
	glm::vec3 up = this->transform.Up();

	this->V = glm::lookAt(pos, pos+forward, up);
	this->P = glm::perspective(this->fovy, this->aspect, near, far);
	this->VP = P * V;
}