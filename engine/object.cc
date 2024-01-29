#include "Object.h"

Data::Data() {
	this->id = 0;
	MemoryManager::Instance().Add(this);
}
Data::~Data() {}


Float::Float() {
	this->value = 0.f;
}
Float::Float(float value) {
	this->value = value;
}
Float::~Float() {}


Vec3::Vec3() {}
Vec3::Vec3(const glm::vec3& value) {
	this->value = value;
}
Vec3::~Vec3() {}


Object::Object() {
	
}
Object::~Object() {}

Data*& Object::Get(const std::string& name) {
	if (this->objects.count(name) == 0) {
		this->objects[name] = nullptr;
	}
	return this->objects[name];
}


MemoryManager::MemoryManager() {}
MemoryManager& MemoryManager::Instance() {
	static MemoryManager instance;
	return instance;
}
MemoryManager::~MemoryManager() {
	for (size_t i = 0; i < this->data.size(); i++) {
		delete this->data[i];
	}
}

void MemoryManager::Add(Data* data) {
	data->id = this->data.size();
	this->data.push_back(data);
}

void MemoryManager::Remove(Data*& data) {
	this->data.erase(this->data.begin() + data->id);
	delete data;
	data = nullptr;
}