#ifndef LITL_CORE_LOGGING_LOGGING_SINK_H__
#define LITL_CORE_LOGGING_LOGGING_SINK_H__

#include <string_view>
#include <thread>

namespace LITL::Core
{
    class LoggingSink
    {
    public:

        LoggingSink();
        virtual ~LoggingSink();

        void enqueue(std::string const& message);

    protected:

        virtual void processMessage(std::string const& message) = 0;

    private:

        void run(std::stop_token stoppingToken);

        struct Impl;
        Impl* m_pImpl;
    };
}

#endif