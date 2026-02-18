#ifndef LITL_CORE_MATH_H__
#define LITL_CORE_MATH_H__

#include <assert.h>
#include <cstdint>

namespace LITL::Math
{
    /// <summary>
    /// Returns the value constrained to the range [min, max] (inclusive).
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="value"></param>
    /// <param name="min"></param>
    /// <param name="max"></param>
    /// <returns></returns>
    template<typename T>
    constexpr static T clamp(T value, T min, T max) noexcept
    {
        return (value < min ? min : value > max ? max : value);
    }

    /// <summary>
    /// Returns the lesser of the two values.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <returns></returns>
    template<typename T>
    constexpr static T minimum(T a, T b) noexcept
    {
        return (a <= b ? a : b);
    }

    /// <summary>
    /// Returns the greater of the two values.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <returns></returns>
    template<typename T>
    constexpr static T maximum(T a, T b) noexcept
    {
        return (a >= b ? a : b);
    }

    /// <summary>
    /// Returns the smallest value >= offset that is aligned to alignment.
    /// Mathematically it is equivalent to: ceil(offset / alignment) * alignment
    /// </summary>
    /// <param name="offset"></param>
    /// <param name="alignment"></param>
    /// <returns></returns>
    size_t alignMemoryOffsetUp(size_t offset, size_t alignment) noexcept
    {
        assert(offset > 0);
        assert(alignment > 0);
        return (offset + alignment - 1) & ~(alignment - 1);
    }
}

#endif