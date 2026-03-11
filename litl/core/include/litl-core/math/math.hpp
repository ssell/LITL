#ifndef LITL_CORE_MATH_H__
#define LITL_CORE_MATH_H__

#include <algorithm>
#include <assert.h>
#include <cmath>

#include "litl-core/math/traits.hpp"

namespace LITL::Math
{
    [[nodiscard]] constexpr float clamp(float value, float min, float max) noexcept
    {
        assert(min <= max);
        return std::clamp(value, min, max);
    }

    [[nodiscard]] constexpr double clamp(double value, double min, double max) noexcept
    {
        assert(min <= max);
        return std::clamp(value, min, max);
    }

    [[nodiscard]] constexpr uint32_t clamp(int32_t value, int32_t min, int32_t max) noexcept
    {
        assert(min <= max);
        return std::clamp(value, min, max);
    }

    [[nodiscard]] constexpr uint32_t clamp(uint32_t value, uint32_t min, uint32_t max) noexcept
    {
        assert(min <= max);
        return std::clamp(value, min, max);
    }

    [[nodiscard]] constexpr float (min)(float a, float b) noexcept
    {
        return (std::min)(a, b);
    }

    [[nodiscard]] constexpr double (min)(double a, double b) noexcept
    {
        return (std::min)(a, b);
    }

    [[nodiscard]] constexpr uint32_t(min)(int32_t a, int32_t b) noexcept
    {
        return (std::min)(a, b);
    }

    [[nodiscard]] constexpr uint32_t(min)(uint32_t a, uint32_t b) noexcept
    {
        return (std::min)(a, b);
    }

    [[nodiscard]] constexpr float (max)(float a, float b) noexcept
    {
        return (std::max)(a, b);
    }

    [[nodiscard]] constexpr double (max)(double a, double b) noexcept
    {
        return (std::max)(a, b);
    }

    [[nodiscard]] constexpr uint32_t (max)(int32_t a, int32_t b) noexcept
    {
        return (std::max)(a, b);
    }

    [[nodiscard]] constexpr uint32_t (max)(uint32_t a, uint32_t b) noexcept
    {
        return (std::max)(a, b);
    }

    [[nodiscard]] constexpr bool fequals(float a, float b, float relEpsilon = Traits<float>::relative_epsilon, float epsilon = Traits<float>::epsilon) noexcept
    {
        // Note we use both an epsilon and relative epsilon value.
        // Absolute epsilon by itself can fail for large numbers where the gap between representable floats is greater than epsilon itself.
        // For larger values, relative epsilon is used to scale the comparison to the magnitude of the values.
        float diff = abs(a - b);

        if (diff <= epsilon)
        {
            return true;
        }

        return diff <= relEpsilon * (max)(abs(a), abs(b));
    }

    [[nodiscard]] constexpr bool fequals(double a, double b, double relEpsilon = Traits<double>::relative_epsilon, double epsilon = Traits<double>::epsilon) noexcept
    {
        // Note we use both an epsilon and relative epsilon value.
        // Absolute epsilon by itself can fail for large numbers where the gap between representable floats is greater than epsilon itself.
        // For larger values, relative epsilon is used to scale the comparison to the magnitude of the values.
        float diff = abs(a - b);

        if (diff <= epsilon)
        {
            return true;
        }

        return diff <= relEpsilon * (max)(abs(a), abs(b));
    }

    [[nodiscard]] constexpr bool isZero(float x, float epsilon = Traits<float>::epsilon) noexcept
    {
        if (std::isnan(x)) { return false; }
        if (!std::isfinite(x)) { return false; }
        return ((x < epsilon) && (x > -epsilon));
    }

    [[nodiscard]] constexpr bool isZero(double x, double epsilon = Traits<double>::epsilon) noexcept
    {
        if (std::isnan(x)) { return false; }
        if (!std::isfinite(x)) { return false; }
        return ((x < epsilon) && (x > -epsilon));
    }

    [[nodiscard]] constexpr bool isZero(int32_t x) noexcept
    {
        if (!std::isfinite(x)) { return false; }
        return x == 0;
    }

    [[nodiscard]] constexpr bool isZero(uint32_t x) noexcept
    {
        if (!std::isfinite(x)) { return false; }
        return x == 0;
    }

    [[nodiscard]] constexpr bool isOne(float x, float relEpsilon = Traits<float>::relative_epsilon, float epsilon = Traits<float>::epsilon) noexcept
    {
        if (std::isnan(x)) { return false; }
        if (!std::isfinite(x)) { return false; }
        return (std::abs(x - 1.0f) < epsilon);
    }

    [[nodiscard]] constexpr bool isOne(double x, double relEpsilon = Traits<double>::relative_epsilon, double epsilon = Traits<double>::epsilon) noexcept
    {
        if (std::isnan(x)) { return false; }
        if (!std::isfinite(x)) { return false; }
        return (std::abs(x - 1.0) < epsilon);
    }

    [[nodiscard]] constexpr bool isOne(int32_t x) noexcept
    {
        if (!std::isfinite(x)) { return false; }
        return x == 1;
    }

    [[nodiscard]] constexpr bool isOne(uint32_t x) noexcept
    {
        if (!std::isfinite(x)) { return false; }
        return x == 1;
    }

    [[nodiscard]] constexpr bool between(float x, float a, float b, bool inclusive = true) noexcept
    {
        assert(a < b);
        return (inclusive ? (x >= a) && (x <= b) : (x > a) && (x < b));
    }

    [[nodiscard]] constexpr bool between(double x, double a, double b, bool inclusive = true) noexcept
    {
        assert(a < b);
        return (inclusive ? (x >= a) && (x <= b) : (x > a) && (x < b));
    }

    [[nodiscard]] constexpr bool between(int32_t x, int32_t a, int32_t b, bool inclusive = true) noexcept
    {
        assert(a < b);
        return (inclusive ? (x >= a) && (x <= b) : (x > a) && (x < b));
    }

    [[nodiscard]] constexpr bool between(uint32_t x, uint32_t a, uint32_t b, bool inclusive = true) noexcept
    {
        assert(a < b);
        return (inclusive ? (x >= a) && (x <= b) : (x > a) && (x < b));
    }

    [[nodiscard]] constexpr bool between(int64_t x, int64_t a, int64_t b, bool inclusive = true) noexcept
    {
        assert(a < b);
        return (inclusive ? (x >= a) && (x <= b) : (x > a) && (x < b));
    }

    [[nodiscard]] constexpr bool between(uint64_t x, uint64_t a, uint64_t b, bool inclusive = true) noexcept
    {
        assert(a < b);
        return (inclusive ? (x >= a) && (x <= b) : (x > a) && (x < b));
    }

    [[nodiscard]] constexpr float abs(float x) noexcept
    {
        return std::fabs(x);
    }

    [[nodiscard]] constexpr double abs(double x) noexcept
    {
        return std::fabs(x);
    }

    [[nodiscard]] constexpr uint32_t abs(int32_t x) noexcept
    {
        return std::abs(x);
    }

    [[nodiscard]] constexpr uint32_t abs(uint32_t x) noexcept
    {
        return x;
    }

    // std::pow(f) is not constexpr, so ours is not either
    [[nodiscard]] float powf(float base, float exponent) noexcept;
    [[nodiscard]] float powf(float base, int32_t exponent) noexcept;
    [[nodiscard]] float powf(float base, uint32_t exponent) noexcept;
    [[nodiscard]] double powd(double base, double exponent) noexcept;
    [[nodiscard]] double powd(double base, int32_t exponent) noexcept;
    [[nodiscard]] double powd(double base, uint32_t exponent) noexcept;
    [[nodiscard]] uint32_t pow(uint32_t base, uint32_t exponent) noexcept;
    [[nodiscard]] uint32_t pow(uint32_t base, int32_t exponent) noexcept;
    [[nodiscard]] int32_t pow(int32_t base, int32_t exponent) noexcept;
    [[nodiscard]] int32_t pow(int32_t base, uint32_t exponent) noexcept;

    [[nodiscard]] constexpr float lerp(float a, float b, float t) noexcept
    {
        return std::lerp(a, b, t);
    }

    [[nodiscard]] constexpr double lerp(double a, double b, double t) noexcept
    {
        return std::lerp(a, b, t);
    }

    [[nodiscard]] constexpr float degreesToRadians(float deg) noexcept
    {
        return (deg * Traits<float>::deg_to_rad);
    }

    [[nodiscard]] constexpr double degreesToRadians(double deg) noexcept
    {
        return (deg * Traits<double>::deg_to_rad);
    }

    [[nodiscard]] constexpr float radiansToDegrees(float rad) noexcept
    {
        return (rad * Traits<float>::rad_to_deg);
    }

    [[nodiscard]] constexpr double radiansToDegrees(double const rad) noexcept
    {
        return (rad * Traits<double>::rad_to_deg);
    }

    [[nodiscard]] constexpr size_t alignMemoryOffsetUp(size_t offset, size_t alignment) noexcept
    {
        /**
         * Example: offset = 13, alignment = 8. returns 16
         *
         *     (offset + alignment - 1) = 20 = 0001 0100
         *     (alignment - 1) = 7 = 0000 0111, but we NOT it so ~7 = 1111 1000
         *
         * Bitwise operation:
         *
         *     0001 0100
         *   & 1111 1000
         *   -----------
         *     0001 0000 = 16
         */

        assert(offset > 0);
        assert(alignment > 0);
        assert((alignment & (alignment - 1)) == 0);     // power of 2

        return (offset + alignment - 1) & ~(alignment - 1);
    }
}

#endif