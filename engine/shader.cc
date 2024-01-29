#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader() {
	this->program = 0;
	this->type = ShaderType::Custom;
}
Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
	this->type = ShaderType::Custom;
	
	std::string vertexShaderString, fragmentShaderString;

	bool loadSuccess =
		this->ReadFile(vertexShaderPath, vertexShaderString) &&
		this->ReadFile(fragmentShaderPath, fragmentShaderString);

	if (!loadSuccess) { return; }

	this->CreateProgram(vertexShaderString, fragmentShaderString);
}
Shader::~Shader() {
	if (this->program != 0) {
		glDeleteProgram(this->program);
	}
}

bool Shader::ReadFile(const std::string& path, std::string& content) {
	std::ifstream file;
	file.open(path);

	if (file.is_open()) {
		std::stringstream stringStream;
		stringStream << file.rdbuf();
		content = stringStream.str();
		file.close();

		return true;
	}
	else {
		std::cout << "could not load shader: " << path << std::endl;
	}

	return false;
}

void Shader::CreateProgram(const std::string& vertexShaderContentString, const std::string& fragmentShaderContentString) {
	const char* vertexShaderContent = vertexShaderContentString.c_str();
	const char* fragmentShaderContent = fragmentShaderContentString.c_str();

	// create and compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLint vertexShaderLength = static_cast<GLint>(vertexShaderContentString.length());
	glShaderSource(vertexShader, 1, &vertexShaderContent, &vertexShaderLength);
	glCompileShader(vertexShader);

	GLint shaderLogSize;
	glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &shaderLogSize);
	if (shaderLogSize > 0)
	{
		GLchar* errorMessage = new GLchar[shaderLogSize];
		glGetShaderInfoLog(vertexShader, shaderLogSize, NULL, errorMessage);
		std::cout << "could not compile vertex shader:\n" << errorMessage << std::endl;
		delete[] errorMessage;
		return;
	}

	// create and compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLint fragmentShaderLength = static_cast<GLint>(fragmentShaderContentString.length());
	glShaderSource(fragmentShader, 1, &fragmentShaderContent, &fragmentShaderLength);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &shaderLogSize);
	if (shaderLogSize > 0)
	{
		GLchar* errorMessage = new GLchar[shaderLogSize];
		glGetShaderInfoLog(fragmentShader, shaderLogSize, NULL, errorMessage);
		std::cout << "could not compile fragment shader:\n" << errorMessage << std::endl;
		delete[] errorMessage;
		return;
	}

	// create and link program
	this->program = glCreateProgram();
	glAttachShader(this->program, vertexShader);
	glAttachShader(this->program, fragmentShader);
	glLinkProgram(this->program);

	glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &shaderLogSize);
	if (shaderLogSize > 0)
	{
		GLchar* errorMessage = new GLchar[shaderLogSize];
		glGetProgramInfoLog(this->program, shaderLogSize, NULL, errorMessage);
		std::cout << "could not link program:\n" << errorMessage << std::endl;
		delete[] errorMessage;
		return;
	}

	// clean up
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::Use() const {
	glUseProgram(this->program);
}

void Shader::StopUsing() const {
	glUseProgram(0);
}

void Shader::AddNameIfNeeded(const std::string& name) {
	if (this->nameToLocation.find(name) == this->nameToLocation.end()) {
		this->nameToLocation[name] = glGetUniformLocation(this->program, name.c_str());
	}
}

void Shader::SetInt(const std::string& name, GLint value) {
	AddNameIfNeeded(name);
	glUniform1i(this->nameToLocation[name], value);
}

void Shader::SetFloat(const std::string& name, GLfloat value) {
	AddNameIfNeeded(name);
	glUniform1f(this->nameToLocation[name], value);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) {
	AddNameIfNeeded(name);
	glUniform3fv(this->nameToLocation[name], 1, &value[0]);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) {
	AddNameIfNeeded(name);
	glUniformMatrix4fv(this->nameToLocation[name], 1, GL_FALSE, &value[0][0]);
}