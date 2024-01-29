#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vec2.hpp>
#include <unordered_map>


struct KeyState {
	bool held;
	bool down;
	bool up;

	KeyState();
};

class Window {
private:
	GLFWwindow* window;
	int width;
	int height;

	std::unordered_map<int, KeyState> keyStates;
	std::unordered_map<int, KeyState> mouseButtonStates;
	glm::vec2 mousePosition;
	glm::vec2 mouseMove;
	bool firstMouseMove;

	Window();

public:
	static Window& Instance();

	void Setup(GLuint width, GLuint height, const char* name, bool fullscreen);
	bool IsOpen();
	void Close();
	void SwapBuffers();
	void GetSize(int& width, int& height);
	float AspectRatio();

	static void Callback_SetKey(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
	static void Callback_SetMouseButton(GLFWwindow* glfwWindow, int button, int action, int mods);
	static void Callback_SetMousePosition(GLFWwindow* glfwWindow, double xPos, double yPos);

	void UpdateInput();
	const KeyState& GetKey(int keyCode);
	const KeyState& GetMouseButton(int buttonCode);
	const glm::vec2& GetMousePosition();
	const glm::vec2& GetMouseMove();
};

namespace Callback {
	void APIENTRY DebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
}