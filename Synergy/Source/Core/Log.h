
#pragma once
#include <spdlog/spdlog.h>


namespace SNG
{

	class Log
	{
	public:

		static void Initialize();
		static std::shared_ptr<spdlog::logger>& GetLogger();
	};
}
