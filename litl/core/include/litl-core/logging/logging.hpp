#ifndef LITL_CORE_LOGGING_LOGGING_H__
#define LITL_CORE_LOGGING_LOGGING_H__

#include "litl-core/logging/logLevel.hpp"

namespace LITL::Core
{
    class Logger
    {
    public:

        static void initialize(char const* logName, bool consoleSink, bool fileSink);
        static void shutdown();

    protected:

    private:

        static void logMessage(LogLevel logLevel, char const* message);
    };
}

#endif