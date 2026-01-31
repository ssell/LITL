#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

#include "litl-core/logger.hpp"

// Just so happens we define the levels inverse of spdlog.
#define TO_SPDLOG_LEVEL(x) (static_cast<spdlog::level::level_enum>(5 - x))

namespace
{
    LITL::Core::LogLevel currentLogLevel = LITL::Core::LogLevel::Debug;
    std::shared_ptr<spdlog::logger> logger;
}

namespace LITL::Core
{
    void Logger::initialize(bool console, bool file)
    {
        logger = std::make_shared<spdlog::logger>("litl", spdlog::sinks_init_list{ 
            std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
            std::make_shared<spdlog::sinks::daily_file_sink_mt>("litl.log")
         });

        logger->set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
        setLogLevel(currentLogLevel);

        spdlog::register_logger(logger);
    }

    void Logger::shutdown()
    {
        if (logger != nullptr)
        {
            spdlog::shutdown();
            logger.reset();
        }
    }

    void Logger::setLogLevel(LogLevel logLevel)
    {
        if (logger != nullptr)
        {
            logger->set_level(TO_SPDLOG_LEVEL(logLevel));
        }
    }

    void Logger::logMessage(LogLevel logLevel, fmt::string_view message, fmt::format_args args)
    {
        if (logger == nullptr)
        {
            return;
        }

        logger->log(TO_SPDLOG_LEVEL(logLevel), message, args);
    }
}