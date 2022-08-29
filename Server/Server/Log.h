#pragma once
#pragma warning(disable:4996)
#include <fstream>
#include <mutex>
#include <spdlog/spdlog.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include <spdlog/sinks/rotating_file_sink.h>

#define log_assert(expression) (void)(																						\
            (!!(expression)) || _assertion_log_error_write(#expression, __FILE__, (unsigned)(__LINE__)) ||	\
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0)								\
        )

inline void LogInit()
{
	shared_ptr<spdlog::logger> consoleLogger = spdlog::stdout_color_mt("console");
	shared_ptr<spdlog::logger> fileLogger = spdlog::rotating_logger_mt("log", "ServerLogs/log.txt", 3 * 1024 * 1024, 100);
	shared_ptr<spdlog::logger> testLogger = spdlog::rotating_logger_mt("test", "ServerLogs/testLog.txt", 3 * 1024 * 1024, 100);
}

template<typename ... Arguments>
inline void LogWrite(const char* format, Arguments... args)
{
	const auto logger = spdlog::get("log");
	logger->info(format, args...);
	logger->flush();
}

template<typename ... Arguments>
inline void LogWriteTest(const char* format, Arguments... args)
{
	const auto logger = spdlog::get("test");
	logger->info(format, args...);
	logger->flush();
}

template<typename ... Arguments>
inline void LogWriteWarning(const char* format, Arguments... args)
{
	const auto logger = spdlog::get("log");
	logger->warn(format, args...);
	logger->flush();
}

template<typename ... Arguments>
inline void LogWriteError(const char* format, Arguments... args)
{
	const auto logger = spdlog::get("log");
	logger->error(format, args...);
	logger->flush();
}

inline bool _assertion_log_error_write(const char* message, const char* file, unsigned int line)
{
	LogWriteError("Assertion failed: {0}, {1}, line {2}", message, file, line);
	return false;
}

template<typename ... Arguments>
void LogPrintf(const char* format, Arguments... args)
{
	spdlog::get("console")->info(format, args...);
}

template<typename ... Arguments>
void Log(const char* format, Arguments... args)
{
	spdlog::get("console")->info(format, args...);
	LogWrite(format, args...);
}

template<typename ... Arguments>
void LogWarning(const char* format, Arguments... args)
{
	spdlog::get("console")->warn(format, args...);
	LogWriteWarning(format, args...);
}
