#ifndef LITL_MATH_COMMON_H__
#define LITL_MATH_COMMON_H__

#include <cstdint>

namespace LITL
{
    template<typename T>
    constexpr static T clamp(T value, T min, T max) noexcept
    {
        return (value < min ? min : value > max ? max : value);
    }

    template<typename T>
    constexpr static T minimum(T a, T b) noexcept
    {
        return (a <= b ? a : b);
    }

    template<typename T>
    constexpr static T maximum(T a, T b) noexcept
    {
        return (a >= b ? a : b);
    }


}

#endif