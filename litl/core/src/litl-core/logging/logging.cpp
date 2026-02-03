#include <vector>
#include <thread>
#include <optional>

#include "litl-core/containers/concurrentQueue.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/logging/sinks/loggingSink.hpp"
#include "litl-core/logging/sinks/consoleSink.hpp"
#include "litl-core/logging/sinks/fileSink.hpp"

namespace LITL::Core
{
    class LogProcessor
    {
    public:

        LogProcessor(bool consoleSink, bool fileSink)
            : m_sinks(),
              m_messageQueue(),
              m_processingThread(std::jthread{ [this](std::stop_token stoppingToken) { this->run(stoppingToken); } })
        {
            if (consoleSink)
            {
                m_sinks.emplace_back(new ConsoleLoggingSink());
            }

            if (fileSink)
            {
                m_sinks.emplace_back(new FileLoggingSink("litl-engine.log"));
            }
        }

        ~LogProcessor()
        {
            m_messageQueue.shutdown();
            m_processingThread.request_stop();

            for (auto iter = m_sinks.begin(); iter != m_sinks.end(); ++iter)
            {
                delete (*iter);
            }

            m_sinks.clear();
        }

        void addSink(LoggingSink* pSink)
        {
            m_sinks.emplace_back(pSink);
        }

        void enqueueMessage(std::string const& message)
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
        ConcurrentQueue<std::string> m_messageQueue;
        std::jthread m_processingThread;
    };

    namespace
    {
        static std::unique_ptr<LogProcessor> pProcessor;
    }

    const std::array<std::string, 7> Logger::LogLevelNames = { "[CRITICAL] ", "[ERROR] ", "[WARNING] ", "[INFO] ", "[DEBUG] ", "[TRACE] ", "[?] " };

    void Logger::initialize(char const* logName, bool consoleSink, bool fileSink)
    {
        if (pProcessor != nullptr)
        {
            return;
        }

        pProcessor = std::make_unique<LogProcessor>(consoleSink, fileSink);
    }

    void Logger::shutdown()
    {

    }

    void Logger::addSink(LoggingSink* sink)
    {
        if (pProcessor != nullptr)
        {
            pProcessor->addSink(sink);
        }
    }

    void Logger::logMessage(LogLevel logLevel, std::string const& message)
    {
        if (pProcessor != nullptr)
        {
            pProcessor->enqueueMessage(message);
        }
    }
}