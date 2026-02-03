#include <vector>

#include "litl-core/containers/concurrentQueue.hpp"
#include <thread>
#include <optional>
#include <string_view>

#include "litl-core/logging/logging.hpp"
#include "litl-core/logging/sinks/loggingSink.hpp"
#include "litl-core/logging/sinks/consoleSink.hpp"

namespace LITL::Core
{
    class LogProcessor
    {
    public:

        LogProcessor(bool consoleSink, bool fileSink)
            : m_sinks(2),
              m_messageQueue(),
              m_processingThread(std::jthread{ [this](std::stop_token stoppingToken) { this->run(stoppingToken); } })
        {
            if (consoleSink)
            {
                m_sinks.emplace_back(new ConsoleLoggingSink());
            }
        }

        ~LogProcessor()
        {
            m_messageQueue.shutdown();
            m_processingThread.request_stop();
        }

        void enqueueMessage(std::string_view message)
        {
            m_messageQueue.enqueue(message);
        }

    protected:

    private:

        void run(std::stop_token stoppingToken)
        {
            while (!stoppingToken.stop_requested())
            {
                auto messageOpt = m_messageQueue.dequeueWait();

                if (messageOpt != std::nullopt)
                {
                    for (auto& sink : m_sinks)
                    {
                        sink->enqueue(messageOpt.value());
                    }
                }
            }
        }

        std::vector<LoggingSink*> m_sinks;
        ConcurrentQueue<std::string_view> m_messageQueue;
        std::jthread m_processingThread;
    };

    namespace
    {
        LogProcessor* pProcessor = nullptr;
    }

    void Logger::initialize(char const* logName, bool consoleSink, bool fileSink)
    {
        if (pProcessor != nullptr)
        {
            return;
        }

        pProcessor = new LogProcessor(consoleSink, fileSink);
    }

    void Logger::shutdown()
    {
        if (pProcessor != nullptr)
        {
            delete pProcessor;
            pProcessor = nullptr;
        }
    }

    void Logger::logMessage(LogLevel logLevel, std::string_view message)
    {
        if (pProcessor != nullptr)
        {
            pProcessor->enqueueMessage(message);
        }
    }
}