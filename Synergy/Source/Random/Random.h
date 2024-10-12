
#pragma once
#include "Core/Core.h"


namespace SNG
{

	class Random
	{
	public:

		static void Initialize();

		static uint32_t UINT();
		static uint32_t UINT(uint32_t min, uint32_t max);
		static float Float();
		static Vec3 Vector3();
		static Vec3 Vector3(float min, float max);

		static uint64_t GenerateUUID();
	};
}
