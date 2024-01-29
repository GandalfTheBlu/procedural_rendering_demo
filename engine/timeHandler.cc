#include "timeHandler.h"

Time::Time() {
	this->currentTime = 0.f;
	this->deltaTime = 0.f;
}

Time::~Time() {}

Time& Time::Instance() {
	static Time instance;
	return instance;
}