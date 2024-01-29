#pragma once
#include "window.h"
#include "camera.h"
#include "modularShader.h"
#include "deferred.h"
#include "object.h"
#include "timeHandler.h"

class Application;

class Game {
public:
	Camera* camera = nullptr;

	Game();
	virtual ~Game();

	virtual void Start() = 0;
	virtual void Update() = 0;
	virtual void Close() = 0;
};

class QuantumTunnelGame : public Game{
public:
	QuantumTunnelGame();
	~QuantumTunnelGame();

	GraphicsObject* ship = nullptr;
	GraphicsObject* voxels = nullptr;
	GraphicsObject* coin = nullptr;
	GraphicsObject* explosion = nullptr;
	GraphicsObject* laser = nullptr;
	float xAngle = 0.f;
	float zAngle = 0.f;
	float sensitivity = 0.2f;
	glm::vec3 vel;
	
	void UpdateLaser();
	void UpdateExplosion();

	void Start();
	void Update();
	void Close();
};

class FirstPersonGame : public Game {
public:
	FirstPersonGame();
	~FirstPersonGame();

	float xAngle = 0.f;
	float zAngle = 0.f;
	float sensitivity = 0.2f;
	Material* invertMaterial = nullptr;
	bool invertSwitch = true;

	void Start();
	void Update();
	void Close();
};

class Application {
public:
	Game* game = nullptr;

	Application();
	
	void Start();
	void Update();
	void Close();
};