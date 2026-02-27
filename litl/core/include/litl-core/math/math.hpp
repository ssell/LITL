#ifndef LITL_CORE_MATH_H__
#define LITL_CORE_MATH_H__

#include <algorithm>
#include <assert.h>
#include <cmath>

namespace LITL::Math
{
    template<typename T>
    struct MathTraits;

    template<>
    struct MathTraits<float>
    {
        static constexpr float epsilon = 0.000001f;
        static constexpr float relative_epsilon = 0.0001f;
        static constexpr float pi = 3.1415926f;
        static constexpr float one_over_pi = 0.31830989f;
        static constexpr float deg_to_rad = 0.0174533f;
        static constexpr float rad_to_deg = 57.2958f;
        static constexpr float phi = 1.6180339887f;
    };

    template<>
    struct MathTraits<double>
    {
        static constexpr float epsilon = 0.0000001;
        static constexpr float relative_epsilon = 0.0001;
        static constexpr float pi = 3.14159265359;
        static constexpr float one_over_pi = 0.31830989;
        static constexpr float deg_to_rad = 0.0174533;
        static constexpr float rad_to_deg = 57.2958;
        static constexpr float phi = 1.6180339887;
    };

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

    [[nodiscard]] constexpr bool fequals(float a, float b, float relEpsilon = MathTraits<float>::relative_epsilon, float epsilon = MathTraits<float>::epsilon) noexcept
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

    [[nodiscard]] constexpr bool fequals(double a, double b, double relEpsilon = MathTraits<double>::relative_epsilon, double epsilon = MathTraits<double>::epsilon) noexcept
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

    [[nodiscard]] constexpr bool isZero(float x, float epsilon = MathTraits<float>::epsilon) noexcept
    {
        if (std::isnan(x)) { return false; }
        if (!std::isfinite(x)) { return false; }
        return ((x < epsilon) && (x > -epsilon));
    }

    [[nodiscard]] constexpr bool isZero(double x, double epsilon = MathTraits<double>::epsilon) noexcept
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

    [[nodiscard]] constexpr bool isOne(float x, float relEpsilon = MathTraits<float>::relative_epsilon, float epsilon = MathTraits<float>::epsilon) noexcept
    {
        if (std::isnan(x)) { return false; }
        if (!std::isfinite(x)) { return false; }
        return (std::abs(x - 1.0f) < epsilon);
    }

    [[nodiscard]] constexpr bool isOne(double x, double relEpsilon = MathTraits<double>::relative_epsilon, double epsilon = MathTraits<double>::epsilon) noexcept
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
        return (deg * MathTraits<float>::deg_to_rad);
    }

    [[nodiscard]] constexpr double degreesToRadians(double deg) noexcept
    {
        return (deg * MathTraits<double>::deg_to_rad);
    }

    [[nodiscard]] constexpr float radiansToDegrees(float rad) noexcept
    {
        return (rad * MathTraits<float>::rad_to_deg);
    }

    [[nodiscard]] constexpr double radiansToDegrees(double const rad) noexcept
    {
        return (rad * MathTraits<double>::rad_to_deg);
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