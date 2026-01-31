#ifndef LITL_CORE_LOGGING_H__
#define LITL_CORE_LOGGING_H__

#include <spdlog/fmt/fmt.h>

namespace LITL::Core
{
    enum LogLevel
    {
        Critical = 0,
        Error = 1,
        Warning = 2,
        Info = 3,
        Debug = 4,
        Trace = 5
    };

    class Logger
    {
    public:

        static void initialize(bool console, bool file);

        static void shutdown();

        /// <summary>
        /// Everything at the specified log level and down will be emitted.
        /// For example: if setting to LogLevel::Debug then everything but trace messages will be reported.
        /// </summary>
        /// <param name="logLevel"></param>
        static void setLogLevel(LogLevel logLevel);

        template<typename... Args>
        static void trace(fmt::format_string<Args...> message, Args&&... args)
        {
            logMessage(LogLevel::Trace, message, fmt::make_format_args(args...));
        }

        template<typename... Args>
        static void debug(fmt::format_string<Args...> message, Args&&... args)
        {
            logMessage(LogLevel::Debug, message, fmt::make_format_args(args...));
        }

        template<typename... Args>
        static void info(fmt::format_string<Args...> message, Args&&... args)
        {
            logMessage(LogLevel::Info, message, fmt::make_format_args(args...));
        }

        template<typename... Args>
        static void warning(fmt::format_string<Args...> message, Args&&... args)
        {
            logMessage(LogLevel::Warning, message, fmt::make_format_args(args...));
        }

        template<typename... Args>
        static void error(fmt::format_string<Args...> message, Args&&... args)
        {
            logMessage(LogLevel::Error, message, fmt::make_format_args(args...));
        }

        template<typename... Args>
        static void critical(fmt::format_string<Args...> message, Args&&... args)
        {
            logMessage(LogLevel::Critical, message, fmt::make_format_args(args...));
        }

    protected:

    private:

        static void logMessage(LogLevel logLevel, fmt::string_view message, fmt::format_args args);
    };
}
#endif