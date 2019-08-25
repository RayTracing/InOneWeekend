#pragma once
/*
	@author: Vassillen Chizhov, August 2019

	random() can serve as a substitute for drand48()

	Addresses Issue #65
*/


#include <random>
#include <array>
#include <cstdint>

class RNG64
{
protected:
	std::mt19937_64 gen;
	std::uniform_real_distribution<float> uniDistF32;
public:
	RNG64() : uniDistF32(0.0f, 1.0f)
	{
		std::random_device randomDevice;
		// initialize the engine with all 312 random numbers (they are twice less than for the 32-bit version since they are 64-bit).
		auto randomVals = std::array<std::mt19937_64::result_type, std::mt19937_64::state_size>{};
		// stitch together 2 random 32-bit values from 
		for (auto& val : randomVals) val = ((static_cast<uint64_t>(randomDevice()) << 32) | static_cast<uint64_t>(randomDevice()));
		std::seed_seq seq(begin(randomVals), end(randomVals));
		gen = std::mt19937_64{ seq };
	}

	float uniform()
	{
		return uniDistF32(gen);
	}
};

float random()
{
	thread_local static RNG64 rng;
	return rng.uniform();
}