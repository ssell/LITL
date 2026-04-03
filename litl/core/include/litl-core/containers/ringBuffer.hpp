#ifndef LITL_CORE_CONTAINERS_RING_BUFFER_H__
#define LITL_CORE_CONTAINERS_RING_BUFFER_H__

#include <array>
#include <cstddef>
#include <concepts>
#include <initializer_list>

namespace LITL::Core
{
    /// <summary>
    /// A fixed-size ring buffer.
    /// It can contain N unique items. Once full, any subsequent calls to push will overwrite the oldest items.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <typeparam name="N"></typeparam>
    template<typename T, size_t N> requires (N > 0)
    class RingBuffer
    {
    public:

        RingBuffer()
            : m_next(0), m_count(0)
        {

        }

        explicit RingBuffer(std::initializer_list<T> seed)
            : m_next(0), m_count(0)
        {
            for (auto& v : seed)
            {
                add(v);
            }
        }

        [[nodiscard]] T const& operator[](size_t index) const noexcept
        {
            assert(index < N);
            size_t start = (m_count < N) ? 0 : m_next;      // if full, m_next points at the oldest index.
            return m_array[(start + index) % N];
        }

        void add(T const& value)
        {
            m_array[m_next] = value;
            m_next = (m_next + 1) % N;
            m_count = (m_count < N) ? m_count + 1 : N;
        }

        [[nodiscard]] size_t size() const noexcept
        {
            return m_count;
        }

        [[nodiscard]] constexpr size_t capacity() const noexcept
        {
            return N;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return m_count == 0;
        }

        [[nodiscard]] bool full() const noexcept
        {
            return m_count == N;
        }

        [[nodiscard]] T sum() const noexcept requires std::is_arithmetic_v<T>
        {
            T total{};

            for (auto i = 0; i < m_count; ++i)
            {
                total += m_array[i];
            }

            return total;
        }

        [[nodiscard]] T min() const noexcept requires std::is_arithmetic_v<T>
        {
            T minimum{};

            if (!empty())
            {
                minimum = m_array[0];

                for (auto i = 1; i < m_count; ++i)
                {
                    minimum = (minimum <= m_array[i]) ? minimum : m_array[i];
                }
            }

            return minimum;
        }

        [[nodiscard]] T max() const noexcept requires std::is_arithmetic_v<T>
        {
            T maximum{};

            if (!empty())
            {
                maximum = m_array[0];

                for (auto i = 1; i < m_count; ++i)
                {
                    maximum = (maximum >= m_array[i]) ? maximum : m_array[i];
                }
            }

            return maximum;
        }

        struct Iterator
        {
            RingBuffer const* buffer;
            size_t index{ 0 };

            T const& operator*() const
            {
                return (*buffer)[index];
            }

            Iterator& operator++()
            {
                index += 1;
                return *this;
            }

            bool operator!=(Iterator const& other) const
            {
                return index != other.index;
            }
        };

        [[nodiscard]] Iterator begin() const
        {
            return { this, 0 };
        }

        [[nodiscard]] Iterator end() const
        {
            return { this, m_count };
        }

    protected:

    private:

        std::array<T, N> m_array{};
        size_t m_next;
        size_t m_count;
    };
}

#endif