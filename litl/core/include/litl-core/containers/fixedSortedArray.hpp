#ifndef LITL_CORE_CONTAINERS_FIXED_SORTED_ARRAY_H__
#define LITL_CORE_CONTAINERS_FIXED_SORTED_ARRAY_H__

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <span>

namespace LITL::Core
{
    /// <summary>
    /// I am what I am.
    /// </summary>
    template<size_t Capacity = 64>
    class FixedSortedArray
    {
    public:

        FixedSortedArray(std::span<uint32_t const> source)
            : m_size(source.size())
        {
            assert(m_size <= Capacity);

            std::copy(source.begin(), source.end(), m_array.begin());
            std::sort(m_array.begin(), m_array.begin() + m_size);

            auto iter = std::unique(m_array.begin(), m_array.begin() + m_size);
            m_size = static_cast<std::size_t>(iter - m_array.begin());
        }

        auto begin() const noexcept
        {
            return m_array.begin();
        }

        auto end() const noexcept
        {
            return m_array.end();
        }

        size_t size() const noexcept
        {
            return m_size;
        }

        constexpr size_t capacity() const noexcept
        {
            return Capacity;
        }

    protected:

    private:

        std::array<uint32_t, Capacity> m_array;
        size_t m_size{ 0 };
    };
}

#endif