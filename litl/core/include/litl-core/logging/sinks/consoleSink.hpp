#ifndef LITL_CORE_LOGGING_CONSOLE_SINK_H__
#define LITL_CORE_LOGGING_CONSOLE_SINK_H__

#include "litl-core/logging/sinks/loggingSink.hpp"

namespace LITL::Core
{
    class ConsoleLoggingSink : public LoggingSink
    {
    public:

    protected:

        void processMessage(std::string_view message) override;

    private:
    };
}

#endif