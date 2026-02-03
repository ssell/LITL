#ifndef LITL_CORE_CONTAINERS_CONCURRENT_QUEUE_H__
#define LITL_CORE_CONTAINERS_CONCURRENT_QUEUE_H__

#include <condition_variable>
#include <mutex>
#include <queue>
#include <optional>

#include "litl-core/impl.hpp"

namespace LITL::Core
{
    /// <summary>
    /// Implementation of a Multiple-Producer, Multiple-Consumer (MPMC) thread-safe queue.
    /// </summary>
    template<typename T>
    class ConcurrentQueue
    {
    public:

        ConcurrentQueue() = default;
        ConcurrentQueue(ConcurrentQueue const&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue const&) = delete;

        void enqueue(T value)
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(value);
            }

            // Notify only one (if any) waiting consumer as there is one new item to process.
            m_condition.notify_one();
        }

        std::optional<T> dequeue()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_queue.empty())
            {
                return std::nullopt;
            }

            T value = std::move(m_queue.front());
            m_queue.pop();

            return value;
        }

        std::optional<T> dequeueWait()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [&] { return m_shouldShutdown || !m_queue.empty(); });       // Blocking wait until either: cv has been notified, shutdown is requested, or queue is no longer empty.

            if (m_shouldShutdown && m_queue.empty())
            {
                return std::nullopt;
            }

            T value = std::move(m_queue.front());
            m_queue.pop();

            return value;
        }

        std::optional<T> peek() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (m_queue.empty())
            {
                return std::nullopt;
            }

            return std::move(m_queue.front());
        }

        size_t size() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.size();
        }

        bool empty() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }

        void clear()
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            while (!m_queue.empty())
            {
                m_queue.pop();
            }
        }

        void shutdown()
        {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_shouldShutdown = true;
            }

            // Notify all (if any) waiting consumers.
            m_condition.notify_all();
        }

    protected:

    private:

        mutable std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<T> m_queue;
        bool m_shouldShutdown = false;
    };
}
#endif