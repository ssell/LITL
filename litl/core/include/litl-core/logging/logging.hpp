#ifndef LITL_CORE_LOGGING_LOGGING_H__
#define LITL_CORE_LOGGING_LOGGING_H__

#include <string>
#include <sstream>

#include "litl-core/logging/logLevel.hpp"

#ifndef LITL_LOG_LEVEL
#define LITL_LOG_LEVEL LITL_LOG_LEVEL_DEBUG
#endif

namespace LITL::Core
{
    class LoggingSink;

    class Logger
    {
    public:

        static void initialize(char const* logName, bool consoleSink, bool fileSink);
        static void shutdown();
        static void addSink(LoggingSink* sink);

        template<typename... Args>
        static void log(LogLevel logLevel, Args&&... args)
        {
            if (LITL_LOG_LEVEL >= static_cast<unsigned>(logLevel))
            {
                formatMessage(logLevel, std::forward<Args>(args)...);
            }
        }

        template<typename... Args>
        static void trace(Args&&... args)
        {
#if LITL_LOG_LEVEL >= LITL_LOG_LEVEL_TRACE
            formatMessage(LogLevel::Trace, std::forward<Args>(args)...);
#endif
        }

        template<typename... Args>
        static void debug(Args&&... args)
        {
#if LITL_LOG_LEVEL >= LITL_LOG_LEVEL_DEBUG
            formatMessage(LogLevel::Debug, std::forward<Args>(args)...);
#endif
        }

        template<typename... Args>
        static void info(Args&&... args)
        {
#if LITL_LOG_LEVEL >= LITL_LOG_LEVEL_INFO
            formatMessage(LogLevel::Info, std::forward<Args>(args)...);
#endif
        }

        template<typename... Args>
        static void warning(Args&&... args)
        {
#if LITL_LOG_LEVEL >= LITL_LOG_LEVEL_WARNING
            formatMessage(LogLevel::Warning, std::forward<Args>(args)...);
#endif
        }

        template<typename... Args>
        static void error(Args&&... args)
        {
#if LITL_LOG_LEVEL >= LITL_LOG_LEVEL_ERROR
            formatMessage(LogLevel::Error, std::forward<Args>(args)...);
#endif
        }

        template<typename... Args>
        static void critical(Args&&... args)
        {
#if LITL_LOG_LEVEL >= LITL_LOG_LEVEL_CRITICAL
            formatMessage(LogLevel::Critical, std::forward<Args>(args)...);
#endif
        }

    protected:

    private:

        template<typename... Args>
        static void formatMessage(LogLevel logLevel, Args&&... args)
        {
            std::ostringstream oss;
            (oss << ... << std::forward<Args>(args));

            logMessage(logLevel, oss.str());
        }

        static void logMessage(LogLevel logLevel, std::string const& message);
    };
}


#endif