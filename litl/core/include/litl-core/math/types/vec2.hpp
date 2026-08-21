#ifndef LITL_MATH_VEC2_H__
#define LITL_MATH_VEC2_H__

#include <array>
#include <cassert>
#include <format>
#include <string>
#include <type_traits>

#include "litl-core/assert.hpp"
#include "litl-core/types.hpp"
#include "litl-core/math/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "litl-core/math/common.hpp"

namespace litl
{
    struct vec2
    {
        constexpr vec2() noexcept : value{ 0.0f, 0.0f } {}
        constexpr explicit vec2(float xy) noexcept : value{ xy, xy } {}
        constexpr explicit vec2(glm::vec2 const& other) noexcept : value(other) {}
        constexpr explicit vec2(std::array<float, 2u> const& arr) : value{ arr[0], arr[1] } {}
        constexpr vec2(float x, float y) noexcept : value{ x, y } {}

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(vec2 const& other) const noexcept
        {
            return fequals(value.x, other.value.x) && fequals(value.y, other.value.y);
        }

        [[nodiscard]] constexpr bool operator==(glm::vec2 const& other) const noexcept
        {
            return fequals(value.x, other.x) && fequals(value.y, other.y);
        }

        // ---------------------------------------------------------------------------------
        // Negation
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec2 operator-() const noexcept
        {
            return vec2{ -value };
        }

        // ---------------------------------------------------------------------------------
        // Addition
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec2 operator+(float scalar) const noexcept
        {
            return vec2{ value + scalar };
        }

        [[nodiscard]] constexpr vec2 operator+(vec2 const& other) const noexcept
        {
            return vec2{ value + other.value };
        }

        [[nodiscard]] constexpr vec2 operator+(glm::vec2 const& other) const noexcept
        {
            return vec2{ value + other };
        }

        constexpr vec2& operator+=(float scalar) noexcept
        {
            value += scalar;
            return *this;
        }

        constexpr vec2& operator+=(vec2 const& other) noexcept
        {
            value += other.value;
            return *this;
        }

        constexpr vec2& operator+=(glm::vec2 const& other) noexcept
        {
            value += other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Subtraction
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec2 operator-(float scalar) const noexcept
        {
            return vec2{ value - scalar };
        }

        [[nodiscard]] constexpr vec2 operator-(vec2 const& other) const noexcept
        {
            return vec2{ value - other.value };
        }

        [[nodiscard]] constexpr vec2 operator-(glm::vec2 const& other) const noexcept
        {
            return vec2{ value - other };
        }

        constexpr vec2& operator-=(float scalar) noexcept
        {
            value -= scalar;
            return *this;
        }

        constexpr vec2& operator-=(vec2 const& other) noexcept
        {
            value -= other.value;
            return *this;
        }

        constexpr vec2& operator-=(glm::vec2 const& other) noexcept
        {
            value -= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Multiplication
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec2 operator*(float scalar) const noexcept
        {
            return vec2{ value * scalar };
        }

        [[nodiscard]] constexpr vec2 operator*(vec2 const& other) const noexcept
        {
            return vec2{ value * other.value };
        }

        [[nodiscard]] constexpr vec2 operator*(glm::vec2 const& other) const noexcept
        {
            return vec2{ value * other };
        }

        constexpr vec2& operator*=(float scalar) noexcept
        {
            value *= scalar;
            return *this;
        }

        constexpr vec2& operator*=(vec2 const& other) noexcept
        {
            value *= other.value;
            return *this;
        }

        constexpr vec2& operator*=(glm::vec2 const& other) noexcept
        {
            value *= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Division
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec2 operator/(float scalar) const noexcept
        {
            LITL_FATAL_ASSERT(!isZero(scalar));
            return vec2{ value / scalar };
        }

        [[nodiscard]] constexpr vec2 operator/(vec2 const& other) const noexcept
        {
            LITL_FATAL_ASSERT(!isZero(other.value.x) && !isZero(other.value.y));
            return vec2{ value / other.value };
        }

        [[nodiscard]] constexpr vec2 operator/(glm::vec2 const& other) const noexcept
        {
            LITL_FATAL_ASSERT(!isZero(other.x) && !isZero(other.y));
            return vec2{ value / other };
        }

        constexpr vec2& operator/=(float scalar) noexcept
        {
            LITL_FATAL_ASSERT(!isZero(scalar));
            value /= scalar;
            return *this;
        }

        constexpr vec2& operator/=(vec2 const& other) noexcept
        {
            LITL_FATAL_ASSERT(!other.isZeroed());
            value /= other.value;
            return *this;
        }

        constexpr vec2& operator/=(glm::vec2 const& other) noexcept
        {
            LITL_FATAL_ASSERT(!isZero(other.x) && !isZero(other.y));
            value /= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Accessors
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr float& x() noexcept
        {
            return value.x;
        }

        [[nodiscard]] constexpr float const& x() const noexcept
        {
            return value.x;
        }

        [[nodiscard]] constexpr float& y() noexcept
        {
            return value.y;
        }

        [[nodiscard]] constexpr float const& y() const noexcept
        {
            return value.y;
        }

        // ---------------------------------------------------------------------------------
        // Factory
        // ---------------------------------------------------------------------------------

        [[nodiscard]] static constexpr vec2 zero() noexcept
        {
            return vec2{ 0.0f, 0.0f };
        }

        // ---------------------------------------------------------------------------------
        // Utility
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr float dot(vec2 other) const noexcept
        {
            return glm::dot(value, other.value);
        }

        [[nodiscard]] constexpr float cross(vec2 other) const noexcept
        {
            return (value.x * other.value.y) - (value.y * other.value.x);
        }

        constexpr void normalize() noexcept
        {
            const float length = glm::length(value);
            value = value / length;
        }

        constexpr void normalizeSafe() noexcept
        {
            const float length = glm::length(value);

            if (!isZero(length))
            {
                value = value / length;
            }
        }

        [[nodiscard]] constexpr vec2 normalized() const noexcept
        {
            const float length = glm::length(value);
            return vec2(value / length);
        }

        [[nodiscard]] constexpr vec2 normalizedSafe() const noexcept
        {
            const float length = glm::length(value);

            if (!isZero(length))
            {
                return vec2(value / length);
            }

            return zero();
        }

        constexpr void setZero() noexcept
        {
            value.x = 0.0f;
            value.y = 0.0f;
        }

        [[nodiscard]] constexpr bool isZeroed() const noexcept
        {
            return isZero(value.x) && isZero(value.y);
        }

        std::string toString() const noexcept
        {
            return std::format("({:.3f},{:.3f})", value.x, value.y);
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr glm::vec2& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::vec2 const& data() const noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr float* dataPtr() noexcept
        {
            return glm::value_ptr(value);
        }

        [[nodiscard]] constexpr float const* dataPtr() const noexcept
        {
            return glm::value_ptr(value);
        }

        [[nodiscard]] std::array<float, 2u> toArray() const noexcept
        {
            return { value.x, value.y };
        }

    private:

        glm::vec2 value{ 0.0f, 0.0f };
    };

    [[nodiscard]] constexpr vec2 min(vec2 a, vec2 b) noexcept
    {
        return vec2{ min(a.x(), b.x()), min(a.y(), b.y()) };
    }

    [[nodiscard]] constexpr vec2 max(vec2 a, vec2 b) noexcept
    {
        return vec2{ max(a.x(), b.x()), max(a.y(), b.y()) };
    }

    [[nodiscard]] constexpr float dot(vec2 a, vec2 b) noexcept
    {
        return a.dot(b);
    }

    [[nodiscard]] constexpr float cross(vec2 a, vec2 b) noexcept
    {
        return a.cross(b);
    }

    [[nodiscard]] constexpr vec2 midpoint(vec2 a, vec2 b) noexcept
    {
        return (a + (b - a) * 0.5f);
    }

    [[nodiscard]] constexpr vec2 normalize(vec2 a) noexcept
    {
        return a.normalized();
    }

    static_assert(std::is_nothrow_copy_constructible_v<vec2>);
    static_assert(std::is_nothrow_move_constructible_v<vec2>);
    static_assert(std::is_nothrow_copy_assignable_v<vec2>);
    static_assert(std::is_nothrow_move_assignable_v<vec2>);
    static_assert(std::is_trivially_copyable_v<vec2>);
}

LITL_REGISTER_TYPE_NAME(litl::vec2)

#endif