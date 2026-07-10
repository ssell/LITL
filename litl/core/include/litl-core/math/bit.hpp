#ifndef LITL_CORE_MATH_BIT_H__
#define LITL_CORE_MATH_BIT_H__

#include <concepts>

namespace litl
{
    template<typename T> requires std::integral<T>
    constexpr void bitSet(T& mask, T position) noexcept
    {
        mask |= (static_cast<T>(1) << position);
    }

    template<typename T> requires std::integral<T>
    constexpr void bitClear(T& mask, T position) noexcept
    {
        mask &= ~(static_cast<T>(1) << position);
    }

    template<typename T> requires std::integral<T>
    [[nodiscard]] constexpr bool bitCheck(T mask, T position) noexcept
    {
        return (mask & (static_cast<T>(1) << position)) != static_cast<T>(0);
    }
}

#endif