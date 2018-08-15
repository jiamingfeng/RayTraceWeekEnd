#pragma once

// random generator
#include <random>
#include <chrono>

class Random
{
public:
	Random():
		generator(std::default_random_engine(unsigned int(std::chrono::system_clock::now().time_since_epoch().count())))
	{
		
	}
	~Random() = default;

	float rSample() {
		std::uniform_real_distribution<float> sampleDist(0.0f, 1.0f);
		return sampleDist(generator);
	}
	float rDiffuse() {
		//std::uniform_real_distribution<float> diffuseDist(-1.0f, 1.0f);
		return diffuseDist(generator);
	}

private:
	std::default_random_engine generator;
	//std::uniform_real_distribution<float> sampleDist = std::uniform_real_distribution<float>(0.0f, 1.0f);
	std::uniform_real_distribution<float> diffuseDist = std::uniform_real_distribution<float>(-1.0f, 1.0f);
};

static Random GlobalRandom;