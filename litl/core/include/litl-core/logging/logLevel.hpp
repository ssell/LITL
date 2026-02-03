#ifndef LITL_CORE_LOGGING_LOG_LEVEL_H__
#define LITL_CORE_LOGGING_LOG_LEVEL_H__

namespace LITL::Core
{
    enum class LogLevel : int
    {
        Critical = 0,
        Error = 1,
        Warning = 2,
        Info = 3,
        Debug = 4,
        Trace = 5,
        None = 6
    };
}

#endif