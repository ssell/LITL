#ifndef LITL_MATH_VEC4_H__
#define LITL_MATH_VEC4_H__

#include <cassert>
#include <format>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "litl-core/math/common.hpp"
#include "litl-core/types.hpp"

namespace litl
{
    struct vec3;

    struct vec4
    {
        constexpr vec4() {}
        constexpr vec4(vec4 const& other) : value(other.value) {}
        constexpr explicit vec4(glm::vec4 const& other) : value(other) {}
        constexpr vec4(float x, float y, float z, float w) : value{ x, y, z, w } {}
        explicit vec4(vec3 const& other);

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(vec4 const& other) const noexcept
        {
            return fequals(value.x, other.value.x) && fequals(value.y, other.value.y) && fequals(value.z, other.value.z) && fequals(value.w, other.value.w);
        }

        [[nodiscard]] constexpr bool operator==(glm::vec4 const& other) const noexcept
        {
            return fequals(value.x, other.x) && fequals(value.y, other.y) && fequals(value.z, other.z) && fequals(value.w, other.w);
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
            assert(!isZero(scalar));
            return vec4{ value / scalar };
        }

        [[nodiscard]] constexpr vec4 operator/(vec4 const& other) const noexcept
        {
            assert(!isZero(other.value.x) && !isZero(other.value.y) && !isZero(other.value.z) && !isZero(other.value.w));
            return vec4{ value / other.value };
        }

        [[nodiscard]] constexpr vec4 operator/(glm::vec4 const& other) const noexcept
        {
            assert(!isZero(other.x) && !isZero(other.y) && !isZero(other.z) && !isZero(other.w));
            return vec4{ value / other };
        }

        constexpr vec4& operator/=(float scalar) noexcept
        {
            assert(!isZero(scalar));
            value /= scalar;
            return *this;
        }

        constexpr vec4& operator/=(vec4 const& other) noexcept
        {
            assert(!isZero(other.value.x) && !isZero(other.value.y) && !isZero(other.value.z) && !isZero(other.value.w));
            value /= other.value;
            return *this;
        }

        constexpr vec4& operator/=(glm::vec4 const& other) noexcept
        {
            assert(!isZero(other.x) && !isZero(other.y) && !isZero(other.z) && !isZero(other.w));
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

        [[nodiscard]] constexpr float lengthSquared() const noexcept
        {
            return (value.x * value.x) + (value.y * value.y) + (value.z * value.z) + (value.w * value.w);
        }

        constexpr void normalize() noexcept
        {
            float length = glm::length(value);
            assert(!isZero(length));
            value = value / length;
        }

        [[nodiscard]] constexpr vec4 normalized() const noexcept
        {
            float length = glm::length(value);
            assert(!isZero(length));
            return vec4(value / length);
        }

        [[nodiscard]] constexpr float dot(vec4 const& other) const noexcept
        {
            return glm::dot(value, other.value);
        }

        constexpr void setZero() noexcept
        {
            value.x = 0.0f;
            value.y = 0.0f;
            value.z = 0.0f;
            value.w = 0.0f;
        }

        [[nodiscard]] constexpr bool isZeroed() const noexcept
        {
            return isZero(value.x) && isZero(value.y) && isZero(value.z) && isZero(value.w);
        }

        const void setIdentity() noexcept
        {
            value.x = 0.0f;
            value.y = 0.0f;
            value.z = 0.0f;
            value.w = 1.0f;
        }

        [[nodiscard]] constexpr bool isIdentity() const noexcept
        {
            return isZero(value.x) && isZero(value.y) && isZero(value.z) && isOne(value.w);
        }

        std::string toString() const noexcept
        {
            return std::format("({:.3f},{:.3f},{:.3f},{:.3f})", value.x, value.y, value.z, value.w);
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec3 xyz() const noexcept
        {
            return vec3{ x(), y(), z() };
        }

        [[nodiscard]] constexpr glm::vec4& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::vec4 const& data() const noexcept
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

    private:

        glm::vec4 value{ 0.0f, 0.0f, 0.0f, 1.0f };
    };
}

REGISTER_TYPE_NAME(litl::vec4)

#endif