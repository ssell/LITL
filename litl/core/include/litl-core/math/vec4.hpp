#ifndef LITL_MATH_VEC3_H__
#define LITL_MATH_VEC3_H__

#include <cassert>
#include <glm/glm.hpp>

#include "litl-core/math/math.hpp"
#include "litl-core/types.hpp"

namespace LITL
{
    struct vec4
    {
        constexpr vec4() {}
        constexpr vec4(vec4 const& other) : value(other.value) {}
        constexpr explicit vec4(glm::vec4 const& other) : value(other) {}
        constexpr vec4(float x, float y, float z, float w) : value{ x, y, z, w } {}

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(vec4 const& other) const noexcept
        {
            return Math::fequals(value.x, other.value.x) && Math::fequals(value.y, other.value.y) && Math::fequals(value.z, other.value.z) && Math::fequals(value.w, other.value.w);
        }

        [[nodiscard]] constexpr bool operator==(glm::vec4 const& other) const noexcept
        {
            return Math::fequals(value.x, other.x) && Math::fequals(value.y, other.y) && Math::fequals(value.z, other.z) && Math::fequals(value.w, other.w);
        }

        // ---------------------------------------------------------------------------------
        // Negation
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec4 operator-() const noexcept
        {
            return vec4{ -value };
        }

        // ---------------------------------------------------------------------------------
        // Addition
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec4 operator+(float scalar) const noexcept
        {
            return vec4{ value + scalar };
        }

        [[nodiscard]] constexpr vec4 operator+(vec4 const& other) const noexcept
        {
            return vec4{ value + other.value };
        }

        [[nodiscard]] constexpr vec4 operator+(glm::vec4 const& other) const noexcept
        {
            return vec4{ value + other };
        }

        constexpr vec4& operator+=(float scalar) noexcept
        {
            value += scalar;
            return *this;
        }

        constexpr vec4& operator+=(vec4 const& other) noexcept
        {
            value += other.value;
            return *this;
        }

        constexpr vec4& operator+=(glm::vec4 const& other) noexcept
        {
            value += other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Subtraction
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec4 operator-(float scalar) const noexcept
        {
            return vec4{ value - scalar };
        }

        [[nodiscard]] constexpr vec4 operator-(vec4 const& other) const noexcept
        {
            return vec4{ value - other.value };
        }

        [[nodiscard]] constexpr vec4 operator-(glm::vec4 const& other) const noexcept
        {
            return vec4{ value - other };
        }

        constexpr vec4& operator-=(float scalar) noexcept
        {
            value -= scalar;
            return *this;
        }

        constexpr vec4& operator-=(vec4 const& other) noexcept
        {
            value -= other.value;
            return *this;
        }

        constexpr vec4& operator-=(glm::vec4 const& other) noexcept
        {
            value -= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Multiplication
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec4 operator*(float scalar) const noexcept
        {
            return vec4{ value * scalar };
        }

        [[nodiscard]] constexpr vec4 operator*(vec4 const& other) const noexcept
        {
            return vec4{ value * other.value };
        }

        [[nodiscard]] constexpr vec4 operator*(glm::vec4 const& other) const noexcept
        {
            return vec4{ value * other };
        }

        constexpr vec4& operator*=(float scalar) noexcept
        {
            value *= scalar;
            return *this;
        }

        constexpr vec4& operator*=(vec4 const& other) noexcept
        {
            value *= other.value;
            return *this;
        }

        constexpr vec4& operator*=(glm::vec4 const& other) noexcept
        {
            value *= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Division
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec4 operator/(float scalar) const noexcept
        {
            assert(!Math::isZero(scalar));
            return vec4{ value / scalar };
        }

        [[nodiscard]] constexpr vec4 operator/(vec4 const& other) const noexcept
        {
            assert(!Math::isZero(other.value.x) && !Math::isZero(other.value.y) && !Math::isZero(other.value.z) && !Math::isZero(other.value.w));
            return vec4{ value / other.value };
        }

        [[nodiscard]] constexpr vec4 operator/(glm::vec4 const& other) const noexcept
        {
            assert(!Math::isZero(other.x) && !Math::isZero(other.y) && !Math::isZero(other.z) && !Math::isZero(other.w));
            return vec4{ value / other };
        }

        constexpr vec4& operator/=(float scalar) noexcept
        {
            assert(!Math::isZero(scalar));
            value /= scalar;
            return *this;
        }

        constexpr vec4& operator/=(vec4 const& other) noexcept
        {
            assert(!Math::isZero(other.value.x) && !Math::isZero(other.value.y) && !Math::isZero(other.value.z) && !Math::isZero(other.value.w));
            value /= other.value;
            return *this;
        }

        constexpr vec4& operator/=(glm::vec4 const& other) noexcept
        {
            assert(!Math::isZero(other.x) && !Math::isZero(other.y) && !Math::isZero(other.z) && !Math::isZero(other.w));
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

        [[nodiscard]] constexpr float& z() noexcept
        {
            return value.z;
        }

        [[nodiscard]] constexpr float const& z() const noexcept
        {
            return value.z;
        }

        [[nodiscard]] constexpr float& w() noexcept
        {
            return value.w;
        }

        [[nodiscard]] constexpr float const& w() const noexcept
        {
            return value.w;
        }

        // ---------------------------------------------------------------------------------
        // Utility
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr float length() const noexcept
        {
            return glm::length(value);
        }

        constexpr void normalize() noexcept
        {
            float length = glm::length(value);
            assert(!Math::isZero(length));
            value = value / length;
        }

        [[nodiscard]] constexpr vec4 normalized() const noexcept
        {
            float length = glm::length(value);
            assert(!Math::isZero(length));
            return vec4(value / length);
        }

        [[nodiscard]] constexpr float dot(vec4 const& other) const noexcept
        {
            return glm::dot(value, other.value);
        }

        constexpr void zero() noexcept
        {
            value.x = 0.0f;
            value.y = 0.0f;
            value.z = 0.0f;
            value.w = 0.0f;
        }

        [[nodiscard]] constexpr bool isZeroed() const noexcept
        {
            return Math::isZero(value.x) && Math::isZero(value.y) && Math::isZero(value.z) && Math::isZero(value.w);
        }

        [[nodiscard]] constexpr bool isIdentity() const noexcept
        {
            return Math::isZero(value.x) && Math::isZero(value.y) && Math::isZero(value.z) && Math::isOne(value.w);
        }

    private:

        glm::vec4 value{ 0.0f, 0.0f, 0.0f, 1.0f };
    };
}

REGISTER_TYPE_NAME(LITL::vec4)

#endif