#ifndef LITL_CORE_LOGGING_LOG_LEVEL_H__
#define LITL_CORE_LOGGING_LOG_LEVEL_H__

#define LITL_LOG_LEVEL_CRITICAL 0
#define LITL_LOG_LEVEL_ERROR    1
#define LITL_LOG_LEVEL_WARNING  2
#define LITL_LOG_LEVEL_INFO     3
#define LITL_LOG_LEVEL_DEBUG    4
#define LITL_LOG_LEVEL_TRACE    5
#define LITL_LOG_LEVEL_NONE     6

namespace LITL::Core
{
    enum LogLevel
    {
        Critical = LITL_LOG_LEVEL_CRITICAL,
        Error    = LITL_LOG_LEVEL_ERROR,
        Warning  = LITL_LOG_LEVEL_WARNING,
        Info     = LITL_LOG_LEVEL_INFO,
        Debug    = LITL_LOG_LEVEL_DEBUG,
        Trace    = LITL_LOG_LEVEL_TRACE,
        None     = LITL_LOG_LEVEL_NONE
    };
}

#endif