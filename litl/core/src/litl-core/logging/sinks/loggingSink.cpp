#include <thread>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <string>

#include "litl-core/logging/sinks/loggingSink.hpp"
#include "litl-core/containers/concurrentSingleQueue.hpp"

namespace LITL::Core
{
    struct LoggingSink::Impl
    {
        ConcurrentSingleQueue<std::string> queue;
        std::mutex mutex;
        std::condition_variable condition;
        bool pendingMessages;
        std::jthread jobThread;
    };

    LoggingSink::LoggingSink()
        : m_pImpl(
            new Impl{
                ConcurrentSingleQueue<std::string>(50),
                std::mutex{},
                std::condition_variable{},
                false,
                std::jthread{[this](std::stop_token stoppingToken) { this->run(stoppingToken); }}
            })
    {

    }

    LoggingSink::~LoggingSink()
    {
        delete m_pImpl;
    }

    void LoggingSink::enqueue(std::string const& message)
    {
        if (m_pImpl->queue.enqueue(message))
        {
            {
                std::lock_guard<std::mutex> lock(m_pImpl->mutex);
                m_pImpl->pendingMessages = true;
            }

            m_pImpl->condition.notify_one();
        }
        // else queue is full!
    }

    void LoggingSink::run(std::stop_token stoppingToken)
    {
        while (!stoppingToken.stop_requested())
        {
            std::optional<std::string> messageOpt{ std::nullopt };

            do
            {
                messageOpt = m_pImpl->queue.dequeue();

                if (messageOpt != std::nullopt)
                {
                    processMessage(messageOpt.value());
                }
            } while (messageOpt != std::nullopt);

            std::unique_lock<std::mutex> lock(m_pImpl->mutex);
            m_pImpl->pendingMessages = false;
            m_pImpl->condition.wait(lock, [&] { return m_pImpl->pendingMessages || stoppingToken.stop_requested(); });
        }
    }
}