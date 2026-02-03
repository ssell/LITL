#ifndef LITL_CORE_LOGGING_LOGGING_H__
#define LITL_CORE_LOGGING_LOGGING_H__

#include <array>
#include <chrono>
#include <format>
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

        static std::string formatFileCapture(char const* file, int line)
        {
            std::ostringstream oss;
            (oss << " [" << file << "@" << line << "]");

            return oss.str();
        }

    protected:

    private:

        static const std::array<std::string, 7> LogLevelNames;

        template<typename... Args>
        static void formatMessage(LogLevel logLevel, Args&&... args)
        {
            const auto now = std::chrono::system_clock::now();

            std::ostringstream oss;
            oss << std::format("[{:%Y-%m-%dT%H:%M:%S}] ", now) << LogLevelNames.at(logLevel);
            (oss << ... << std::forward<Args>(args));

            logMessage(logLevel, oss.str());
        }

        static void logMessage(LogLevel logLevel, std::string const& message);
    };
}

namespace LITL
{
    /**
     * Note here we prefer a free forwarding convenience function instead of macros.
     * A macro such as the one below suffers from several problems.
     *
     *     #define LITL_LOG_TRACE(...) LITL::Core::Logger::trace(__VA_ARGS__)
     *
     * 1. No type safety or scope awareness. Macros are text substitution.
     * 2. Loss of overload resolution. If logger is updated to support std::format as an overload, then the macro can not resolve the target.
     * 3. Debugging is worse. Breakpoints go to logger, not the call site, and errors reference the macro body not the user code.
     */

    template<typename... Args>
    inline void logTrace(Args&&... args)
    {
        LITL::Core::Logger::trace(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void logDebug(Args&&... args)
    {
        LITL::Core::Logger::debug(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void logInfo(Args&&... args)
    {
        LITL::Core::Logger::info(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void logWarning(Args&&... args)
    {
        LITL::Core::Logger::warning(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void logError(Args&&... args)
    {
        LITL::Core::Logger::error(std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void logCritical(Args&&... args)
    {
        LITL::Core::Logger::critical(std::forward<Args>(args)...);
    }
}

// Macro here only for file/line capture. Use other options if file/line capture is not needed.

#define LITL_LOG_TRACE_CAPTURE(...)    LITL::Core::Logger::trace(__VA_ARGS__, LITL::Core::Logger::formatFileCapture(__FILE__, __LINE__))
#define LITL_LOG_DEBUG_CAPTURE(...)    LITL::Core::Logger::debug(__VA_ARGS__, LITL::Core::Logger::formatFileCapture(__FILE__, __LINE__))
#define LITL_LOG_INFO_CAPTURE(...)     LITL::Core::Logger::info(__VA_ARGS__, LITL::Core::Logger::formatFileCapture(__FILE__, __LINE__))
#define LITL_LOG_WARNING_CAPTURE(...)  LITL::Core::Logger::warning(__VA_ARGS__, LITL::Core::Logger::formatFileCapture(__FILE__, __LINE__))
#define LITL_LOG_ERROR_CAPTURE(...)    LITL::Core::Logger::error(__VA_ARGS__, LITL::Core::Logger::formatFileCapture(__FILE__, __LINE__))
#define LITL_LOG_CRITICAL_CAPTURE(...) LITL::Core::Logger::critical(__VA_ARGS__, LITL::Core::Logger::formatFileCapture(__FILE__, __LINE__))

#endif