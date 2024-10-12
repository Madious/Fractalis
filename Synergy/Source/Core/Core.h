
#pragma once
#include "Snpch.h"
#include "Log.h"


#ifndef SNG_PLATFORM_WINDOWS
	#error Synergy Engine only support Windows platform for now
#endif


#ifdef SNG_DEBUG
	#define SNG_ASSERT(x) if (!(x)) __debugbreak();
	#define VULKAN_VALIDATION_LAYERS 1
	#define SNG_CHECK_VULKAN_RESULT(x, ...) if ((x) != VK_SUCCESS) SNG_ERROR(__VA_ARGS__);
#else
	#define SNG_ASSERT(x)
	#define VULKAN_VALIDATION_LAYERS 0
	#define SNG_CHECK_VULKAN_RESULT(x, ...) x;
#endif


#define SNG_TRACE(...) ::SNG::Log::GetLogger()->trace(__VA_ARGS__);
#define SNG_INFO(...) ::SNG::Log::GetLogger()->info(__VA_ARGS__);
#define SNG_WARN(...) ::SNG::Log::GetLogger()->warn(__VA_ARGS__);
#define SNG_ERROR(...) ::SNG::Log::GetLogger()->error(__VA_ARGS__);
#define SNG_FATAL(...) ::SNG::Log::GetLogger()->critical(__VA_ARGS__);


#define SNG_BIT(x) 1 << x;
#define SNG_BIND_FUNC(func, numParam) std::bind(&func, this, std::placeholders::_##numParam)
#define SNG_IS_BASE_OF(Ty, Ba) std::is_base_of<Ba, Ty>().value


namespace SNG
{

	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Weak = std::weak_ptr<T>;


	template<typename T, typename... Args>
	Ref<T> MakeRef(Args... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	Scope<T> MakeScope(Args... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}



	using Vec2 = glm::vec2;
	using Vec3 = glm::vec3;
	using Vec4 = glm::vec4;
	using Mat4 = glm::mat4;
}
