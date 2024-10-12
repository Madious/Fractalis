
#include "Snpch.h"
#include "Log.h"

#include <vector>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>


namespace SNG
{

	static std::shared_ptr<spdlog::logger> s_Logger;


	void Log::Initialize()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Log/Synergy.txt", true));
		logSinks[0]->set_pattern("%^[%T] (%l): %v%$");
		logSinks[1]->set_pattern("[%T] (%l): %v");

		s_Logger = std::make_shared<spdlog::logger>("Synergy", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_Logger);
		s_Logger->set_level(spdlog::level::trace);
		s_Logger->flush_on(spdlog::level::trace);
	}

	std::shared_ptr<spdlog::logger>& Log::GetLogger()
	{
		return s_Logger;
	}
}
