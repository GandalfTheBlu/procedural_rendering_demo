#pragma once

class Time {
private:
	Time();

public:
	~Time();

	static Time& Instance();

	float currentTime;
	float deltaTime;
};