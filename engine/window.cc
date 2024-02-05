#include "window.h"
#include <iostream>

KeyState::KeyState() {
    this->held = false;
    this->down = false;
    this->up = false;
}

Window::Window() {
	this->window = nullptr;
    this->width = 0;
    this->height = 0;
    this->firstMouseMove = true;
    this->mouseMove = glm::vec2(0.f);
    this->mousePosition = glm::vec2(0.f);
}

Window& Window::Instance() {
    static Window instance;
    return instance;
}

void Window::Setup(GLuint width, GLuint height, const char* name, bool fullscreen) {
    if (glfwInit() != GLFW_TRUE)
    {
        std::cout << "glfw failed to load" << std::endl;
    }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    if (fullscreen) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        this->width = mode->width;
        this->height = mode->height;
    }
    else {
        this->width = width;
        this->height = height;
    }

    this->window = glfwCreateWindow(this->width, this->height, name, nullptr, nullptr);
	if (this->window == nullptr) {
		std::cout << "glfw failed to create window" << std::endl;
	}
	glfwMakeContextCurrent(this->window);

	if (glewInit() != GLEW_OK) {
		std::cout << "glew failed to load" << std::endl;
	}

    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(Callback::DebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
	glViewport(0, 0, this->width, this->height);

    glfwSetKeyCallback(this->window, Window::Callback_SetKey);
    glfwSetMouseButtonCallback(this->window, Window::Callback_SetMouseButton);
    glfwSetCursorPosCallback(this->window, Window::Callback_SetMousePosition);
}

bool Window::IsOpen() {
	return !glfwWindowShouldClose(this->window);
}

void Window::Close() {
	glfwTerminate();
}

void Window::SwapBuffers() {
	glfwSwapBuffers(this->window);
}

void Window::GetSize(int& width, int& height) {
    width = this->width;
    height = this->height;
}

float Window::AspectRatio() {
    return (float)this->width / this->height;
}

void Window::Callback_SetKey(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods) {
    KeyState& keyState = Window::Instance().keyStates[key];

    switch (action) {
    case GLFW_PRESS:
        keyState.down = true;
        keyState.held = true;
        break;
    case GLFW_RELEASE:
        keyState.held = false;
        keyState.up = true;
        break;
    }
}

void Window::Callback_SetMouseButton(GLFWwindow* glfwWindow, int button, int action, int mods) {
    KeyState& keyState = Window::Instance().mouseButtonStates[button];

    switch (action) {
    case GLFW_PRESS:
        keyState.down = true;
        keyState.held = true;
        break;
    case GLFW_RELEASE:
        keyState.held = false;
        keyState.up = true;
        break;
    }
}

void Window::Callback_SetMousePosition(GLFWwindow* glfwWindow, double xPos, double yPos) {
    glm::vec2 newPos(xPos, yPos);

    if (Window::Instance().firstMouseMove) {
        Window::Instance().firstMouseMove = false;
        Window::Instance().mousePosition = newPos;
    }
    else {
        Window::Instance().mouseMove = newPos - Window::Instance().mousePosition;
        Window::Instance().mousePosition = newPos;
    }
}

void Window::UpdateInput() {

    for (auto& pair : this->keyStates) {
        KeyState& keyState = this->keyStates[pair.first];
        keyState.down = false;
        keyState.up = false;
    }

    for (auto& pair : this->mouseButtonStates) {
        KeyState& keyState = this->mouseButtonStates[pair.first];
        keyState.down = false;
        keyState.up = false;
    }

    this->mouseMove = glm::vec2();

    glfwPollEvents();
}

const KeyState& Window::GetKey(int keyCode) {
    if (this->keyStates.find(keyCode) != this->keyStates.end()) {
        return this->keyStates[keyCode];
    }
    return KeyState();
}

const KeyState& Window::GetMouseButton(int buttonCode) {
    if (this->mouseButtonStates.find(buttonCode) != this->mouseButtonStates.end()) {
        return this->mouseButtonStates[buttonCode];
    }
    return KeyState();
}

const glm::vec2& Window::GetMousePosition() {
    return this->mousePosition;
}

const glm::vec2& Window::GetMouseMove() {
    return this->mouseMove;
}

namespace Callback {
    void APIENTRY DebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam)
    {
        // ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        std::cout << "---------------" << std::endl;
        std::cout << "Debug message (" << id << "): " << message << std::endl;

        switch (source)
        {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
        } std::cout << std::endl;

        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
        } std::cout << std::endl;

        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
        } std::cout << std::endl;
        std::cout << std::endl;
    }
}