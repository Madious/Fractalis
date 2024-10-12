
#include "Snpch.h"
#include "Random.h"


namespace SNG
{

	static thread_local std::mt19937 s_RandomEngine;
	static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	static thread_local std::mt19937_64 s_RandomEngine64;
	static std::uniform_int_distribution<std::mt19937_64::result_type> s_Distribution64;


	void Random::Initialize()
	{
		s_RandomEngine.seed(std::random_device()());
		s_RandomEngine64.seed(std::random_device()());
	}

	uint32_t Random::UINT()
	{
		return s_Distribution(s_RandomEngine);
	}

	uint32_t Random::UINT(uint32_t min, uint32_t max)
	{
		return min + (s_Distribution(s_RandomEngine) % (max - min + 1));
	}

	float Random::Float()
	{
		return s_Distribution(s_RandomEngine) / FLT_MAX;
	}

	Vec3 Random::Vector3()
	{
		return { Random::Float(), Random::Float(), Random::Float() };
	}

	Vec3 Random::Vector3(float min, float max)
	{
		return {
			Random::Float() * (max - min) + min,
			Random::Float() * (max - min) + min,
			Random::Float() * (max - min) + min,
		};
	}

	uint64_t Random::GenerateUUID()
	{
		return s_Distribution64(s_RandomEngine64);
	}
}
