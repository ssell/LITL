#ifndef LITL_CORE_CONTAINERS_CONCURRENT_SINGLE_QUEUE_H__
#define LITL_CORE_CONTAINERS_CONCURRENT_SINGLE_QUEUE_H__

#include <atomic>
#include <cstddef>
#include <vector>

namespace LITL::Core::Containers
{
    /// <summary>
    /// Implementation of a Single-Producer, Single-Consumer (SPSC) thread-safe queue.
    /// Note that a SPSC queue is implemented via a ring buffer and is bounded in it's capacity, unlike a MPMC queue.
    /// </summary>
    template<typename T>
    class ConcurrentSingleQueue
    {
    public:

        explicit ConcurrentSingleQueue(uint32_t capacity)
            : m_capacity(capacity), m_buffer(capacity + 1)
        {

        }

        bool push(T const& value)
        {
            auto writeAt = m_writeAt.load(std::memory_order_relaxed);
            auto next = increment(writeAt);

            if (next == m_readAt.load(std::memory_order_acquire))
            {
                return false;               // queue is full
            }

            m_buffer[writeAt] = value;
            m_writeAt.store(next, std::memory_order_release);

            return true;
        }

        std::optional<T> pop()
        {
            auto readAt = m_readAt.load(std::memory_order_relaxed);

            if (readAt == m_writeAt.load(std::memory_order_acquire))
            {
                return std::nullopt;        // queue is empty
            }

            T value = std::move(m_buffer[readAt]);
            m_readAt.store(increment(readAt), std::memory_order_release);

            return value;
        }

        std::optional<T> peek()
        {
            auto readAt = m_readAt.load(std::memory_order_relaxed);

            if (readAt == m_writeAt.load(std::memory_order_acquire))
            {
                return std::nullopt;        // queue is empty
            }

            T value = std::move(m_buffer[readAt]);
            return value;
        }

        uint32_t size() const
        {
            const auto writeAt = m_writeAt.load(std::memory_order_acquire);
            const auto readAt = m_readAt.load(std::memory_order_acquire);

            if (writeAt >= readAt)
            {
                return (writeAt - readAt);
            }
            else
            {
                return (m_capacity - readAt) + writeAt;
            }
        }

        uint32_t capacity() const
        {
            return m_capacity;
        }

        bool empty() const
        {
            return (size() == 0);
        }

        bool full() const
        {
            return (size() == m_capacity);
        }

        void clear()
        {
            m_writeAt.store(0, std::memory_order_release);
            m_readAt.store(0, std::memory_order_release);
        }

    protected:

    private:

        uint32_t increment(uint32_t curr) const
        {
            return (curr + 1) % m_buffer.size();
        }

        const uint32_t m_capacity;
        std::atomic<uint32_t> m_writeAt;        // aka head
        std::atomic<uint32_t> m_readAt;         // aka tail
        std::vector<T> m_buffer;
        // ^ both always moving "forward", with the tail chasing the head
    };
}

#endif