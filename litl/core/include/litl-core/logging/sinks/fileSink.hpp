#ifndef LITL_CORE_LOGGING_FILE_SINK_H__
#define LITL_CORE_LOGGING_FILE_SINK_H__

#include "litl-core/logging/sinks/loggingSink.hpp"

namespace LITL::Core
{
    class FileLoggingSink : public LoggingSink
    {
    public:

        FileLoggingSink(const char* logName);

    protected:

        void processMessage(std::string const& message) override;

    private:

        std::string m_logFileName;
    };
}

#endif