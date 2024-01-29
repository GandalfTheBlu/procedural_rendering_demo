#pragma once
#include "object.h"
#include <glm.hpp>
#include "transform.h"


class Camera : public Object{
public:
	struct FrustumPlane {
	public:
		glm::vec3 normal;
		float offset;

		FrustumPlane();
		FrustumPlane(const glm::vec3& normal, float offset);
	};

	struct Frustum {
	public:
		FrustumPlane near;
		FrustumPlane far;
		FrustumPlane left;
		FrustumPlane right;
		FrustumPlane bottom;
		FrustumPlane top;

		FrustumPlane* operator[](const int i);

		Frustum();
	};

	Transform transform;
	glm::mat4x4 V;
	glm::mat4x4 P;
	glm::mat4x4 VP;
	float fovy;
	float aspect;
	float near;
	float far;

	Frustum frustum;

	Camera();
	Camera(float fovy, float aspect, float near, float far);
	~Camera();

	
	void CalcFrustum();
	void CalcMatrices();
};