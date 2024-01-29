#include "application.h"
#include <gtx/transform.hpp>
#include <iostream>
#include "voxelData.h"



Game::Game(){}
Game::~Game() {}

QuantumTunnelGame::QuantumTunnelGame(){}
QuantumTunnelGame::~QuantumTunnelGame(){}

void QuantumTunnelGame::Start() {
	ModularShader* caveShader = new ModularShader(ShaderType::ProceduralVoxels, "shaders/custom/caveVoxels.glsl");
	Material* caveMaterial = new Material(
		caveShader,
		[](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetFloat("u_time", Time::Instance().currentTime);
			glm::vec3 shipPos = _get(obj, Vec3, "shipPos")->value;
			shader->SetVec3("u_shipPos", shipPos);
		}
	);
	Mesh* screenQuad = new Mesh();
	Mesh::ScreenQuad(*screenQuad);
	GraphicsObject* caveObject = new GraphicsObject(screenQuad, { caveMaterial });
	caveObject->Get("shipPos") = new Vec3(glm::vec3(0.f));
	this->voxels = caveObject;
	Renderer::Instance().Add(caveObject);

	ModularShader* shipShader = new ModularShader(ShaderType::SDFVolume, "shaders/custom/ship.glsl");
	Material* shipMaterial = new Material(
		shipShader,
		[](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetFloat("u_time", Time::Instance().currentTime);
			float speed = _get(obj, Float, "speed")->value;
			shader->SetFloat("u_speed", speed);
		}
	);
	Mesh* shipMesh = new Mesh();
	Mesh::Cube(1.f, 3.f, 1.f, *shipMesh);
	GraphicsObject* shipObject = new GraphicsObject(shipMesh, { shipMaterial });
	shipObject->Get("speed") = new Float(0.f);
	this->ship = shipObject;
	Renderer::Instance().Add(shipObject);

	ModularShader* coinShader = new ModularShader(ShaderType::SDFSurface, "shaders/custom/coin.glsl");
	Material* coinMaterial = new Material(
		coinShader,
		[](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetFloat("u_time", Time::Instance().currentTime);
		}
	);
	Mesh* cube = new Mesh();
	Mesh::Cube(2.f, 2.f, 2.f, *cube);
	GraphicsObject* coinObject = new GraphicsObject(cube, { coinMaterial });
	coinObject->transform.position.y = 60.f;
	this->coin = coinObject;
	Renderer::Instance().Add(coinObject);


	ModularShader* explosionShader = new ModularShader(ShaderType::BoundSDF, "shaders/custom/explosion.glsl");
	Material* explosionMaterial = new Material(
		explosionShader,
		[](GraphicsObject* obj, Shader* shader, Camera& cam) {
			float timer = _get(obj, Float, "timer")->value;
			shader->SetFloat("u_timer", timer);
		}
	);
	GraphicsObject* explosionObject = new GraphicsObject(screenQuad, { explosionMaterial });
	explosionObject->transform.position = glm::vec3(3.f);
	explosionObject->transform.scale = glm::vec3(10.f);
	explosionObject->Get("timer") = new Float(-1.f);
	this->explosion = explosionObject;
	Renderer::Instance().Add(explosionObject);

	ModularShader* laserShader = new ModularShader(ShaderType::SDFVolume, "shaders/custom/laser.glsl");
	Material* laserMaterial = new Material(
		laserShader,
		[](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetFloat("u_time", Time::Instance().currentTime);
			float isShooting = _get(obj, Float, "isShooting")->value;
			shader->SetFloat("u_isShooting", isShooting);
		}
	);
	Mesh* laserMesh = new Mesh();
	Mesh::Cube(0.25f, 30.f, 0.25f, *laserMesh);
	GraphicsObject* laserObject = new GraphicsObject(laserMesh, { laserMaterial });
	laserObject->Get("isShooting") = new Float(0.f);
	this->laser = laserObject;
	Renderer::Instance().Add(laserObject);

	this->laser->transform.ChangeParent(&this->ship->transform);
	this->laser->transform.position = glm::vec3(0.f, 31.f, 0.f);

	Shader* skyBoxShader = new Shader("shaders/raycast/commonVertex.glsl", "shaders/custom/skyboxFragment.glsl");
	Material* skyBoxMaterial = new Material(
		skyBoxShader,
		[](GraphicsObject* _nullptr, Shader* shader, Camera& camera) {
			shader->SetFloat("u_aspect", camera.aspect);
			shader->SetFloat("u_tan_half_fovy", glm::tan(0.5f * camera.fovy));
			shader->SetFloat("u_near", camera.near);
			shader->SetVec3("u_up", camera.transform.Up());
			shader->SetVec3("u_forward", camera.transform.Forward());

			shader->SetVec3("u_dirLight", Renderer::Instance().directionalLight->direction);
			shader->SetVec3("u_ambientColor", Renderer::Instance().directionalLight->ambientColor);
		}
	);
	Renderer::Instance().skyBoxMaterial = skyBoxMaterial;

	Shader* fadeShader = new Shader("shaders/deferred/full_screen_vertex.glsl", "shaders/custom/tunnelPostPro.glsl");
	Material* fadeMaterial = new Material(
		fadeShader,
		[](GraphicsObject* _nullptr, Shader* shader, Camera& camera) {
			Renderer::Instance().BindGPosition(1);
			shader->SetVec3("u_camPos", camera.transform.WorldPosition());
		}
	);
	fadeMaterial->CleanUpAfterDraw = []() {
		Renderer::Instance().UnbindGTexture(1);
	};
	Renderer::Instance().AddPostProcessingMaterial(fadeMaterial);

	Renderer::Instance().directionalLight->ambientColor = glm::vec3(0.05f);
	Renderer::Instance().directionalLight->direction = glm::normalize(glm::vec3(-0.5f, 0.8f, -1.f));
	Renderer::Instance().directionalLight->intensity = 0.5f;

	Renderer::Instance().spotLights.push_back(new SpotLight(glm::vec3(), glm::vec3(0.f, 1.f, 0.f), glm::vec3(1.f), glm::vec3(0.001f, 0.02f, 1.f), 1.2f));
	Renderer::Instance().spotLights[0]->transform.ChangeParent(&this->ship->transform);
	Renderer::Instance().spotLights[0]->transform.position = glm::vec3(0.f, 2.f, 0.f);

	this->camera = new Camera(3.141592f * 80.f / 180.f, Window::Instance().AspectRatio(), 0.3f, 500.f);
}

void QuantumTunnelGame::Update() {
	glm::vec3 move(0.f);
	float moveSpeed = 35.f;

	if (Window::Instance().GetKey(GLFW_KEY_W).held) {
		move.y += 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_S).held) {
		move.y -= 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_A).held) {
		move.x -= 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_D).held) {
		move.x += 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_Q).held) {
		move.z += 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_E).held) {
		move.z -= 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_SPACE).held) {
		moveSpeed = 60.f;
	}
	if (Window::Instance().GetMouseButton(GLFW_MOUSE_BUTTON_2).held) {
		glm::vec2 mouseMove = Window::Instance().GetMouseMove() * Time::Instance().deltaTime * this->sensitivity;
		this->xAngle -= mouseMove.y;
		this->zAngle -= mouseMove.x;
	}
	if (Window::Instance().GetKey(GLFW_KEY_R).down) {
		this->ship->transform.position = glm::vec3(0.5f, 0.f, 0.5f);
		this->vel = glm::vec3(0.f);
	}

	glm::vec3 forward = this->ship->transform.Forward();
	glm::vec3 worldMove = forward * glm::max(move.y, 0.f);

	this->camera->transform.rotation =
		glm::angleAxis(this->zAngle, glm::vec3(0.f, 0.f, 1.f)) *
		glm::angleAxis(this->xAngle, glm::vec3(1.f, 0.f, 0.f));

	glm::vec3 axis = this->camera->transform.rotation * glm::vec3(0.f, 1.f, 0.f);

	float r = 10.f + 0.1 * glm::length(this->vel);
	this->camera->transform.position = -r * axis + this->ship->transform.position;

	if (glm::dot(worldMove, worldMove) != 0.f) {
		glm::fquat goalRot = this->camera->transform.rotation * glm::angleAxis(0.2f, glm::vec3(1.f, 0.f, 0.f));

		if (!Window::Instance().GetKey(GLFW_KEY_LEFT_SHIFT).held) {
			this->ship->transform.rotation = glm::slerp(this->ship->transform.rotation, goalRot, 0.1f);
		}
		this->vel = glm::mix(vel, worldMove * moveSpeed, 0.1);
	}

	this->ship->transform.position += this->vel * Time::Instance().deltaTime;
	this->vel *= 1.f - 0.2 * Time::Instance().deltaTime;
	_get(this->ship, Float, "speed")->value = glm::length(this->vel);
	_get(this->voxels, Vec3, "shipPos")->value = ship->transform.position;

	this->UpdateLaser();
	this->UpdateExplosion();
}

void QuantumTunnelGame::Close() {}

void QuantumTunnelGame::UpdateLaser() {
	if (Window::Instance().GetMouseButton(GLFW_MOUSE_BUTTON_1).held) {
		_get(this->laser, Float, "isShooting")->value = 1.f;

		bool missCoin = true;
		glm::vec3 coinDir = this->coin->transform.position - this->ship->transform.position;
		float coinDirProjOnAim = glm::dot(this->ship->transform.Forward(), coinDir);

		if (coinDirProjOnAim >= 0.f) {
			float aimOffsetFromCoin = glm::sqrt(glm::dot(coinDir, coinDir) - coinDirProjOnAim * coinDirProjOnAim);

			if (aimOffsetFromCoin < 2.f) {
				this->explosion->transform.position = this->coin->transform.position - this->explosion->transform.scale * 0.5f;
				_get(this->explosion, Float, "timer")->value = 0.f;

				this->coin->transform.position.y += 120.f;
				float angle = Time::Instance().currentTime + this->coin->transform.position.x * 100.f;
				this->coin->transform.position.x = 15.f * glm::cos(angle);
				this->coin->transform.position.z = 15.f * glm::sin(angle);

				missCoin = false;
			}
		}
	}
	else {
		_get(this->laser, Float, "isShooting")->value = 0.f;
	}
}

void QuantumTunnelGame::UpdateExplosion() {
	Float* timer = _get(this->explosion, Float, "timer");
	if (timer->value >= 0.f) {
		timer->value += Time::Instance().deltaTime * 1.5f * glm::exp(-timer->value);
	}
	if (timer->value >= 1.f) {
		timer->value = -1.f;
	}
}


FirstPersonGame::FirstPersonGame(){}
FirstPersonGame::~FirstPersonGame(){}

void FirstPersonGame::Start(){
	/*ModularShader* boundSDFShader = new ModularShader(ShaderType::BoundSDF, "shaders/custom/boundSDFTest.glsl");
	Material* boundSDFMaterial = new Material(
		boundSDFShader
	);
	Mesh* screenQuad = new Mesh();
	Mesh::ScreenQuad(*screenQuad);
	GraphicsObject* boundSDFObject = new GraphicsObject(screenQuad, {boundSDFMaterial});
	boundSDFObject->transform.scale = glm::vec3(2.f);
	boundSDFObject->transform.position = glm::vec3(-1.f, 1.f, -1.f);
	Renderer::Instance().Add(boundSDFObject);

	ModularShader* infiniteSDFShader = new ModularShader(ShaderType::InfiniteSDF, "shaders/custom/infSDFTest.glsl");
	Material* infiniteSDFMaterial = new Material(
		infiniteSDFShader,
		[](GraphicsObject* obj, Shader* shader, Camera& cam) {
			shader->SetFloat("u_time", Time::Instance().currentTime);
		}
	);
	GraphicsObject* infiniteSDFObject = new GraphicsObject(screenQuad, {infiniteSDFMaterial});
	Renderer::Instance().Add(infiniteSDFObject);

	ModularShader* voxelShader = new ModularShader(ShaderType::ProceduralVoxels, "shaders/custom/voxelProcTest.glsl");
	Material* voxelMaterial = new Material(
		voxelShader
	);
	GraphicsObject* voxelObject = new GraphicsObject(screenQuad, {voxelMaterial});
	Renderer::Instance().Add(voxelObject);*/

	ModularShader* surfaceSDFShader1 = new ModularShader(ShaderType::SDFSurface, "shaders/custom/surfaceSDFTest2.glsl");
	Material* surfaceSDFMaterial1 = new Material(
		surfaceSDFShader1
	);
	ModularShader* surfaceSDFShader2 = new ModularShader(ShaderType::SDFSurface, "shaders/custom/surfaceSDFTest3.glsl");
	Material* surfaceSDFMaterial2 = new Material(
		surfaceSDFShader2
	);
	Mesh* cubeMesh = new Mesh();
	Mesh::Cube(1.f, 1.f, 1.f, *cubeMesh);

	GraphicsObject* surfaceSDFObject1 = new GraphicsObject(cubeMesh, { surfaceSDFMaterial1 });
	Renderer::Instance().Add(surfaceSDFObject1);
	surfaceSDFObject1->transform.position = glm::vec3(0.f, 2.f, 0.f);

	GraphicsObject* surfaceSDFObject2 = new GraphicsObject(cubeMesh, { surfaceSDFMaterial2 });
	Renderer::Instance().Add(surfaceSDFObject2);
	surfaceSDFObject2->transform.position = glm::vec3(4.f, 2.f, 0.f);

	Shader* skyBoxShader = new Shader("shaders/raycast/commonVertex.glsl", "shaders/custom/skyboxFragment.glsl");
	Material* skyBoxMaterial = new Material(
		skyBoxShader,
		[](GraphicsObject* _nullptr, Shader* shader, Camera& camera) {
			shader->SetFloat("u_aspect", camera.aspect);
			shader->SetFloat("u_tan_half_fovy", glm::tan(0.5f * camera.fovy));
			shader->SetFloat("u_near", camera.near);
			shader->SetVec3("u_up", camera.transform.Up());
			shader->SetVec3("u_forward", camera.transform.Forward());

			shader->SetVec3("u_dirLight", Renderer::Instance().directionalLight->direction);
			shader->SetVec3("u_ambientColor", Renderer::Instance().directionalLight->ambientColor);
		}
	);
	Renderer::Instance().skyBoxMaterial = skyBoxMaterial;

	/*Shader* flatShader = new Shader("shaders/deferred/flat_vertex.glsl", "shaders/deferred/flat_fragment.glsl");
	Material* flatMaterial = new Material(
		flatShader,
		[](GraphicsObject* obj, Shader* shader, Camera& camera) {
			glm::mat4 MVP = camera.VP * obj->transform.matrix;
			shader->SetMat4("u_MVP", MVP);
			shader->SetVec3("u_color", glm::vec3(1.f,0.5f,0.5f));
		}
	);
	flatMaterial->renderType = Material::RenderType::Forward;
	GraphicsObject* cubeObject = new GraphicsObject(
		cubeMesh, {flatMaterial}
	);
	cubeObject->transform.position = glm::vec3(3.f, 5.f, 5.f);
	Renderer::Instance().Add(cubeObject);


	Shader* toneShader = new Shader("shaders/deferred/full_screen_vertex.glsl", "shaders/custom/effectsFragment.glsl");
	Material* toneMaterial = new Material(toneShader);
	Renderer::Instance().AddPostProcessingMaterial(toneMaterial);

	Shader* invertShader = new Shader("shaders/deferred/full_screen_vertex.glsl", "shaders/custom/effectsFragment2.glsl");
	this->invertMaterial = new Material(invertShader);
	Renderer::Instance().AddPostProcessingMaterial(this->invertMaterial);*/

	Renderer::Instance().directionalLight->ambientColor = glm::vec3(0.45f, 0.84f, 1.f);
	Renderer::Instance().directionalLight->lightColor = glm::vec3(1.f, 0.95f, 0.8f);
	Renderer::Instance().directionalLight->direction = glm::normalize(glm::vec3(-0.6f, 0.3f, -1.f));
	Renderer::Instance().directionalLight->intensity = 1.f;
	Renderer::Instance().debugLightSources = true;

	Renderer::Instance().pointLights.push_back(new PointLight(glm::vec3(-1.5f, 2.f, -1.f), glm::vec3(1.f, 0.6f, 1.f), glm::vec3(0.8f, 0.5f, 1.f)));
	Renderer::Instance().pointLights.push_back(new PointLight(glm::vec3(1.5f, 1.5f, 2.5f), glm::vec3(1.f, 0.2f, 0.8f), glm::vec3(0.01f, 0.1f, 1.f)));
	Renderer::Instance().spotLights.push_back(new SpotLight(glm::vec3(-1.f, -1.f, 1.f), glm::normalize(glm::vec3(1.f, 1.f, -1.f)), glm::vec3(1.f,0.2f,0.2f), glm::vec3(0.01f,0.1f,1.f), 1.2f));

	this->camera = new Camera(3.141592f * 80.f / 180.f, Window::Instance().AspectRatio(), 0.3f, 500.f);
}

void FirstPersonGame::Update() {
	glm::vec3 move(0.f);
	float moveSpeed = 3.f;

	if (Window::Instance().GetKey(GLFW_KEY_W).held) {
		move.y += 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_S).held) {
		move.y -= 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_A).held) {
		move.x -= 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_D).held) {
		move.x += 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_Q).held) {
		move.z += 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_E).held) {
		move.z -= 1.f;
	}
	if (Window::Instance().GetKey(GLFW_KEY_SPACE).held) {
		moveSpeed = 5.f;
	}
	if (Window::Instance().GetMouseButton(GLFW_MOUSE_BUTTON_2).held) {
		glm::vec2 mouseMove = Window::Instance().GetMouseMove() * Time::Instance().deltaTime * this->sensitivity;
		this->xAngle = glm::clamp(this->xAngle + mouseMove.y, -3.141592f * 0.49f, 3.141592f * 0.49f);
		this->zAngle += mouseMove.x;
	}
	/*if (Window::Instance().GetKey(GLFW_KEY_R).down) {
		if (this->invertSwitch) {
			Renderer::Instance().RemovePostProcessingMaterial(this->invertMaterial);
		}
		else {
			Renderer::Instance().AddPostProcessingMaterial(this->invertMaterial);
		}
		this->invertSwitch = !this->invertSwitch;
	}*/

	this->camera->transform.rotation =
		glm::angleAxis(this->zAngle, glm::vec3(0.f, 0.f, 1.f)) *
		glm::angleAxis(this->xAngle, glm::vec3(1.f, 0.f, 0.f));

	glm::vec3 right = this->camera->transform.Right();
	glm::vec3 forward = this->camera->transform.Forward();
	glm::vec3 up = this->camera->transform.Up();
	glm::vec3 worldMove = right * move.x + forward * move.y + up * move.z;

	if (glm::dot(worldMove, worldMove) != 0.f) {
		this->camera->transform.position += glm::normalize(worldMove) * Time::Instance().deltaTime * moveSpeed;
	}

	this->camera->transform.rotation =
		glm::angleAxis(this->zAngle, glm::vec3(0.f, 0.f, 1.f)) *
		glm::angleAxis(this->xAngle, glm::vec3(1.f, 0.f, 0.f));
}

void FirstPersonGame::Close() {}


Application::Application() {}

void Application::Start() {
	Window::Instance().Setup(1280, 720, "Raygon", false);
	int width, height;
	Window::Instance().GetSize(width, height);
	Renderer::Instance().Setup(width, height);

	//this->game = new QuantumTunnelGame();
	this->game = new FirstPersonGame();
	this->game->Start();
}

void Application::Update() {
	double t0 = glfwGetTime();
	float timeSinceStart = 0.f;

	while (Window::Instance().IsOpen()) {
		double t1 = glfwGetTime();
		float deltaTime = (float)(t1 - t0);
		t0 = t1;
		timeSinceStart += deltaTime;
		Time::Instance().currentTime = timeSinceStart;
		Time::Instance().deltaTime = deltaTime;

		Window::Instance().UpdateInput();
		this->game->Update();
		TransformHandler::Instance().root->Update();
		this->game->camera->CalcMatrices();

		Renderer::Instance().Draw(*this->game->camera);

		Window::Instance().SwapBuffers();
	}
}

void Application::Close() {
	this->game->Close();
	delete this->game;

	Window::Instance().Close();
}

