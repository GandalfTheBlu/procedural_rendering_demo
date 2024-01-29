#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <vec3.hpp>

#define _get(obj, type, name) dynamic_cast<type*>(obj->Get(name))

class Data {
private:
	size_t id;

	friend class MemoryManager;

public:
	Data();
	virtual ~Data();
};

class Float : public Data {
public:
	float value;

	Float();
	Float(float value);
	~Float();
};

class Vec3 : public Data {
public:
	glm::vec3 value;

	Vec3();
	Vec3(const glm::vec3& value);
	~Vec3();
};

class Object : public Data{
private:
	std::unordered_map<std::string, Data*> objects;

public:

	Object();
	virtual ~Object();

	Data*& Get(const std::string& name);
};

class MemoryManager {
private:
	std::vector<Data*> data;

	MemoryManager();

public:
	static MemoryManager& Instance();
	~MemoryManager();

	void Add(Data* data);
	void Remove(Data*& data);
};