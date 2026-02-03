#ifndef LITL_CORE_LOGGING_LOGGING_H__
#define LITL_CORE_LOGGING_LOGGING_H__

#include <string_view>
#include "litl-core/logging/logLevel.hpp"

namespace LITL::Core
{
    class Logger
    {
    public:

        static void initialize(char const* logName, bool consoleSink, bool fileSink);
        static void shutdown();

        static void log(LogLevel logLevel, std::string_view message)
        {
            logMessage(logLevel, message);
        }

    protected:

    private:

        static void logMessage(LogLevel logLevel, std::string_view message);
    };
}

#endif