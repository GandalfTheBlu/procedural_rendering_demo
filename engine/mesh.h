#pragma once
#include "object.h"
#include <GL/glew.h>
#include <vector>

struct VertexAttribute {
public:
	GLuint location;
	GLint components;
	GLenum componentType;
	GLsizei byteStride;
	const void* byteOffset;

	VertexAttribute();
	VertexAttribute(GLuint location, GLint components, GLenum componentType, GLsizei byteStride, const void* byteOffset);

	void Enable() const;
	void Disable() const;
};

struct IndexAttribute {
public:
	GLenum type;
	GLsizei count;
	const void* byteOffset;

	IndexAttribute();
	IndexAttribute(GLenum type, GLsizei count, const void* byteOffset);

	void DrawIndices() const;
};

struct BufferData {
public:
	const void* dataStart;
	GLsizeiptr byteSize;

	BufferData();
	BufferData(const void* dataStart, GLsizeiptr byteSize);

	void SetData(GLenum bufferType) const;
};

class Mesh : public Object{
private:
	GLuint vertexArrayObject;
	GLuint indexBufferObject;
	std::vector<IndexAttribute> indexAttributes;
	std::vector<GLuint> attributeBufferObjects;

public:
	Mesh();
	~Mesh();

	void Setup(const BufferData& indexData, const std::vector<IndexAttribute>& indexAttributes, const std::vector<BufferData>& vertexData, const std::vector<VertexAttribute>& vertexAttributes);
	void Bind() const;
	void Draw(GLuint subMeshIndex) const;
	void Unbind() const;
	size_t SubMeshCount();

	static void Cube(float x, float y, float z, Mesh& mesh);
	static void Arc(Mesh& mesh);
	static void ScreenQuad(Mesh& mesh);
	static void LightSphere(const float radius, const int resX, const int resY, Mesh& mesh);
	static void LightCone(const float radius, const float height, const int res, Mesh& mesh);
};