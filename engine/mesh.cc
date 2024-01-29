#include "mesh.h"
#include <glm.hpp>

VertexAttribute::VertexAttribute() {
	this->location = 0;
	this->components = 0;
	this->componentType = GL_FLOAT;
	this->byteStride = 0;
	this->byteOffset = (const void*)0;
}

VertexAttribute::VertexAttribute(GLuint location, GLint components, GLenum componentType, GLsizei byteStride, const void* byteOffset) {
	this->location = location;
	this->components = components;
	this->componentType = componentType;
	this->byteStride = byteStride;
	this->byteOffset = byteOffset;
}

void VertexAttribute::Enable() const {
	glEnableVertexAttribArray(this->location);
	glVertexAttribPointer(this->location, this->components, this->componentType, GL_FALSE, this->byteStride, this->byteOffset);
}

void VertexAttribute::Disable() const {
	glDisableVertexAttribArray(this->location);
}


IndexAttribute::IndexAttribute() {
	this->type = GL_UNSIGNED_BYTE;
	this->count = 0;
	this->byteOffset = (const void*)0;
}

IndexAttribute::IndexAttribute(GLenum type, GLsizei count, const void* byteOffset) {
	this->type = type;
	this->count = count;
	this->byteOffset = byteOffset;
}

void IndexAttribute::DrawIndices() const {
	glDrawElements(GL_TRIANGLES, this->count, this->type, this->byteOffset);
}

BufferData::BufferData() {
	this->dataStart = (const void*)0;
	this->byteSize = 0;
}

BufferData::BufferData(const void* dataStart, GLsizeiptr byteSize) {
	this->dataStart = dataStart;
	this->byteSize = byteSize;
}

void BufferData::SetData(GLenum bufferType) const {
	glBufferData(bufferType, this->byteSize, this->dataStart, GL_STATIC_DRAW);
}


Mesh::Mesh() {
	this->vertexArrayObject = 0;
	this->indexBufferObject = 0;
}

Mesh::~Mesh() {
	if (this->vertexArrayObject != 0) {
		glDeleteVertexArrays(1, &this->vertexArrayObject);
	}
	if (this->indexBufferObject != 0) {
		glDeleteBuffers(1, &this->indexBufferObject);
	}
	if (this->attributeBufferObjects.size() > 0) {
		glDeleteBuffers(this->attributeBufferObjects.size(), &this->attributeBufferObjects[0]);
	}
}

void Mesh::Setup(const BufferData& indexData, const std::vector<IndexAttribute>& indexAttributes, const std::vector<BufferData>& vertexData, const std::vector<VertexAttribute>& vertexAttributes) {
	
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);

	glGenBuffers(1, &this->indexBufferObject);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->indexBufferObject);
	indexData.SetData(GL_ELEMENT_ARRAY_BUFFER);
	this->indexAttributes = indexAttributes;
	
	for (int i = 0; i < vertexData.size(); i++) {
		GLuint attributeBufferObject = 0;
		glGenBuffers(1, &attributeBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, attributeBufferObject);
		vertexData[i].SetData(GL_ARRAY_BUFFER);
		this->attributeBufferObjects.push_back(attributeBufferObject);

		if (vertexData.size() == 1) {
			for (int j = 0; j < vertexAttributes.size(); j++) {
				vertexAttributes[j].Enable();
			}
		}
		else {
			vertexAttributes[i].Enable();
		}
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	for (int i = 0; i < vertexAttributes.size(); i++) {
		vertexAttributes[i].Disable();
	}
}

void Mesh::Bind() const {
	glBindVertexArray(this->vertexArrayObject);
}

void Mesh::Draw(GLuint subMeshIndex) const {
	this->indexAttributes[subMeshIndex].DrawIndices();
}

void Mesh::Unbind() const {
	glBindVertexArray(0);
}

size_t Mesh::SubMeshCount() {
	return this->indexAttributes.size();
}

void Mesh::Cube(float x, float y, float z, Mesh& mesh) {

	std::vector<GLfloat> positions{
		// left (0,1,2,3)
		-x,y,-z,
		-x,y,z,
		-x,-y,z,
		-x,-y,-z,
		// right (4,5,6,7)
		x,-y,-z,
		x,-y,z,
		x,y,z,
		x,y,-z,
		// back (8,9,10,11)
		-x,-y,-z,
		-x,-y,z,
		x,-y,z,
		x,-y,-z,
		// forward (12,13,14,15)
		x,y,-z,
		x,y,z,
		-x,y,z,
		-x,y,-z,
		// down (16,17,18,19)
		-x,y,-z,
		-x,-y,-z,
		x,-y,-z,
		x,y,-z,
		// up (20,21,22,23)
		-x,-y,z,
		-x,y,z,
		x,y,z,
		x,-y,z
	};

	std::vector<GLfloat> normals{
		// left (0,1,2,3)
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		// right (4,5,6,7)
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		// back (8,9,10,11)
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		// forward (12,13,14,15)
		0.f, 1.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 1.f, 0.f,
		// down (16,17,18,19)
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		0.f, 0.f, -1.f,
		// up (20,21,22,23)
		0.f, 0.f, 1.f,
		0.f, 0.f, 1.f,
		0.f, 0.f, 1.f,
		0.f, 0.f, 1.f
	};

	std::vector<GLfloat> tangents{
		// left (0,1,2,3)
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		0.f, -1.f, 0.f,
		// right (4,5,6,7)
		0.f, 1.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 1.f, 0.f,
		0.f, 1.f, 0.f,
		// back (8,9,10,11)
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		// forward (12,13,14,15)
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		-1.f, 0.f, 0.f,
		// down (16,17,18,19)
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		// up (20,21,22,23)
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f,
		1.f, 0.f, 0.f
	};

	std::vector<GLfloat> uvs{
		// left (0,1,2,3)
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,
		// right (4,5,6,7)
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,
		// back (8,9,10,11)
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,
		// forward (12,13,14,15)
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,
		// down (16,17,18,19)
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f,
		// up (20,21,22,23)
		0.f, 0.f,
		0.f, 1.f,
		1.f, 1.f,
		1.f, 0.f
	};

	std::vector<GLbyte> indices{
		// left
		0,3,1,
		3,2,1,
		// right
		4,7,5,
		7,6,5,
		// back
		8,11,9,
		11,10,9,
		// forward
		12,15,13,
		15,14,13,
		// down
		16,19,17,
		19,18,17,
		// up
		20,23,21,
		23,22,21
	};

	BufferData positionData(&positions[0], positions.size() * sizeof(GLfloat));
	VertexAttribute positionAttribute(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData normalData(&normals[0], normals.size() * sizeof(GLfloat));
	VertexAttribute normalAttribute(1, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData tangentData(&tangents[0], tangents.size() * sizeof(GLfloat));
	VertexAttribute tangentAttribute(2, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData uvData(&uvs[0], uvs.size() * sizeof(GLfloat));
	VertexAttribute uvAttribute(3, 2, GL_FLOAT, 2 * sizeof(GLfloat), (const void*)0);

	BufferData indexData(&indices[0], indices.size() * sizeof(GLubyte));
	IndexAttribute indexAttribute(GL_UNSIGNED_BYTE, indices.size(), (const void*)0);

	mesh.Setup(indexData, { indexAttribute }, { positionData, normalData, tangentData, uvData}, { positionAttribute , normalAttribute, tangentAttribute, uvAttribute});
}

void Mesh::Arc(Mesh& mesh) {
	std::vector<GLfloat> positions{
		0.f,-1.f,0.f, 0.f,-1.f,2.f, 1.f,-1.f,4.f, 1.f,-1.f,4.f, 3.f,-1.f,6.f, 5.f,-1.f,7.f,
		0.f,1.f,0.f,  0.f,1.f,2.f,  1.f,1.f,4.f,  1.f,1.f,4.f,  3.f,1.f,6.f,  5.f,1.f,7.f
	};

	glm::vec3 n1 = glm::normalize(glm::vec3(-7.f, 0.f, 2.f));
	glm::vec3 n2 = glm::normalize(glm::vec3(-6.f, 0.f, 4.f));
	glm::vec3 n3 = glm::normalize(glm::vec3(-4.f, 0.f, 6.f));
	glm::vec3 n4 = glm::normalize(glm::vec3(-2.f, 0.f, 7.f));

	std::vector<GLfloat> normals{
		-1.f,0.f,0.f, n1.x,n1.y,n1.z, n2.x,n2.y,n2.z, n2.x,n2.y,n2.z, n3.x,n3.y,n3.z, n4.x,n4.y,n4.z,
		-1.f,0.f,0.f, n1.x,n1.y,n1.z, n2.x,n2.y,n2.z, n2.x,n2.y,n2.z, n3.x,n3.y,n3.z, n4.x,n4.y,n4.z
	};

	std::vector<GLfloat> tangents{
		0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f,
		0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f, 0.f,-1.f,0.f
	};

	std::vector<GLfloat> uvs{
		1.f,0.f, 1.f,0.5f, 1.f,1.f, 1.f,0.f, 1.f,0.5f, 1.f,1.f,
		0.f,0.f, 0.f,0.5f, 0.f,1.f, 0.f,0.f, 0.f,0.5f, 0.f,1.f
	};

	std::vector<GLubyte> indices{
		6,0,7, 0,1,7, 7,1,8, 1,2,8, 9,3,10, 3,4,10, 10,4,11, 4,5,11
	};

	BufferData positionData(&positions[0], positions.size() * sizeof(GLfloat));
	VertexAttribute positionAttribute(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData normalData(&normals[0], normals.size() * sizeof(GLfloat));
	VertexAttribute normalAttribute(1, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData tangentData(&tangents[0], tangents.size() * sizeof(GLfloat));
	VertexAttribute tangentAttribute(2, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData uvData(&uvs[0], uvs.size() * sizeof(GLfloat));
	VertexAttribute uvAttribute(3, 2, GL_FLOAT, 2 * sizeof(GLfloat), (const void*)0);

	BufferData indexData(&indices[0], indices.size() * sizeof(GLubyte));
	IndexAttribute indexAttribute(GL_UNSIGNED_BYTE, indices.size(), (const void*)0);

	mesh.Setup(indexData, { indexAttribute }, { positionData, normalData, tangentData, uvData }, { positionAttribute , normalAttribute, tangentAttribute, uvAttribute });
}

void Mesh::ScreenQuad(Mesh& mesh) {
	std::vector<GLfloat> positions{
		-1.f, -1.f,
		-1.f, 1.f, 
		1.f, 1.f, 
		1.f, -1.f, 
	};

	std::vector<GLubyte> indices{
		0, 3, 1,
		1, 3, 2
	};

	BufferData positionData(&positions[0], positions.size() * sizeof(GLfloat));
	VertexAttribute positionAttribute(0, 2, GL_FLOAT, 2 * sizeof(GLfloat), (const void*)0);

	BufferData indexData(&indices[0], indices.size() * sizeof(GLubyte));
	IndexAttribute indexAttribute(GL_UNSIGNED_BYTE, indices.size(), (const void*)0);

	mesh.Setup(indexData, { indexAttribute }, { positionData}, {positionAttribute});
}

void Mesh::LightSphere(const float radius, const int resX, const int resY, Mesh& mesh) {

	const int vertexCount = 6 * (resX + 1) * (resY + 1);
	const int indexCount = 36 * resX * resY;
	const int positionDataCount = vertexCount * 3;

	std::vector<GLuint> indices;
	std::vector<GLfloat> positions;

	indices.reserve(indexCount);
	positions.reserve(positionDataCount);

	// face order: left, right, back, forward, down, up
	glm::vec3 faceCorners[6]{
		glm::vec3(-1.f, 1.f, -1.f),
		glm::vec3(1.f, -1.f, -1.f),
		glm::vec3(-1.f, -1.f, -1.f),
		glm::vec3(1.f, 1.f, -1.f),
		glm::vec3(-1.f, 1.f, -1.f),
		glm::vec3(-1.f, -1.f, 1.f)
	};
	// right and up vector for each face
	glm::vec3 faceAxis[12]{
		glm::vec3(0.f, -1.f, 0.f),
		glm::vec3(0.f, 0.f, 1.f),

		glm::vec3(0.f, 1.f, 0.f),
		glm::vec3(0.f, 0.f, 1.f),

		glm::vec3(1.f, 0.f, 0.f),
		glm::vec3(0.f, 0.f, 1.f),

		glm::vec3(-1.f, 0.f, 0.f),
		glm::vec3(0.f, 0.f, 1.f),

		glm::vec3(1.f, 0.f, 0.f),
		glm::vec3(0.f, -1.f, 0.f),

		glm::vec3(1.f, 0.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f)
	};

	// construct faces
	for (int n = 0; n < 6; n++) {
		// create grid
		int faceStartIndex = n * (resX + 1) * (resY + 1);
		for (int i = 0; i < resX + 1; i++) {
			for (int j = 0; j < resY + 1; j++) {
				GLfloat u = (GLfloat)i / resX;
				GLfloat v = (GLfloat)j / resY;
				glm::vec3 gridPoint = faceCorners[n] + faceAxis[2 * n] * (2.f * u) + faceAxis[2 * n + 1] * (2.f * v);
				glm::vec3 normal = glm::normalize(gridPoint);
				glm::vec3 vertex = normal * radius;


				positions.push_back(vertex.x);
				positions.push_back(vertex.y);
				positions.push_back(vertex.z);

				// create indicis on the fly (not if on right- or top edge)
				if (i < resX && j < resY) {
					GLuint i00 = faceStartIndex + i * (resY + 1) + j;
					GLuint i01 = faceStartIndex + i * (resY + 1) + j + 1;
					GLuint i11 = faceStartIndex + (i + 1) * (resY + 1) + j + 1;
					GLuint i10 = faceStartIndex + (i + 1) * (resY + 1) + j;

					indices.push_back(i10);
					indices.push_back(i01);
					indices.push_back(i00);

					indices.push_back(i11);
					indices.push_back(i01);
					indices.push_back(i10);

				}
			}
		}
	}

	BufferData positionData(&positions[0], positions.size() * sizeof(GLfloat));
	VertexAttribute positionAttribute(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData indexData(&indices[0], indices.size() * sizeof(GLuint));
	IndexAttribute indexAttribute(GL_UNSIGNED_INT, indices.size(), (const void*)0);

	mesh.Setup(indexData, { indexAttribute }, { positionData }, { positionAttribute });
}

void Mesh::LightCone(const float radius, const float height, const int res, Mesh& mesh) {
	const int vertexCount = res + 2;
	const int indexCount = (res * 2) * 3;
	const int positionDataCount = vertexCount * 3;

	std::vector<GLuint> indices;
	std::vector<GLfloat> positions;

	indices.reserve(indexCount);
	positions.reserve(positionDataCount);

	// circle center at index 0
	positions.push_back(0.f);
	positions.push_back(height);
	positions.push_back(0.f);

	// pointy end at index 1
	positions.push_back(0.f);
	positions.push_back(0.f);
	positions.push_back(0.f);

	// construct circle at base
	for (int i = 0; i < res + 1; i++) {
		float angle = (float)i / res * 2.f * 3.141592f;
		float x = glm::cos(angle);
		float z = glm::sin(angle);

		int index0 = positions.size() / 3;

		positions.push_back(radius * x);
		positions.push_back(height);
		positions.push_back(radius * z);

		// add indices on the fly
		if (i < res) {
			// add the triangle that connects to the circle center
			indices.push_back(index0);
			indices.push_back(0);
			indices.push_back(index0 + 1);

			// add the triangle that connects to the pointy end
			indices.push_back(index0);
			indices.push_back(index0 + 1);
			indices.push_back(1);
		}
	}

	BufferData positionData(&positions[0], positions.size() * sizeof(GLfloat));
	VertexAttribute positionAttribute(0, 3, GL_FLOAT, 3 * sizeof(GLfloat), (const void*)0);

	BufferData indexData(&indices[0], indices.size() * sizeof(GLuint));
	IndexAttribute indexAttribute(GL_UNSIGNED_INT, indices.size(), (const void*)0);

	mesh.Setup(indexData, { indexAttribute }, { positionData }, { positionAttribute });
}