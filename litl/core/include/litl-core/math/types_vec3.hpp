#ifndef LITL_MATH_VEC3_H__
#define LITL_MATH_VEC3_H__

#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "litl-core/math/math.hpp"
#include "litl-core/types.hpp"

namespace LITL
{
    struct vec4;

    struct vec3
    {
        constexpr vec3() {}
        constexpr vec3(vec3 const& other) : value(other.value) {}
        constexpr explicit vec3(glm::vec3 const& other) : value(other) {}
        constexpr vec3(float x, float y, float z) : value{ x, y, z } {}
        explicit vec3(vec4 const& other);

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(vec3 const& other) const noexcept
        {
            return Math::fequals(value.x, other.value.x) && Math::fequals(value.y, other.value.y) && Math::fequals(value.z, other.value.z);
        }

        [[nodiscard]] constexpr bool operator==(glm::vec3 const& other) const noexcept
        {
            return Math::fequals(value.x, other.x) && Math::fequals(value.y, other.y) && Math::fequals(value.z, other.z);
        }

        // ---------------------------------------------------------------------------------
        // Negation
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec3 operator-() const noexcept
        {
            return vec3{ -value };
        }

        // ---------------------------------------------------------------------------------
        // Addition
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec3 operator+(float scalar) const noexcept
        {
            return vec3{ value + scalar };
        }

        [[nodiscard]] constexpr vec3 operator+(vec3 const& other) const noexcept
        {
            return vec3{ value + other.value };
        }

        [[nodiscard]] constexpr vec3 operator+(glm::vec3 const& other) const noexcept
        {
            return vec3{ value + other };
        }

        constexpr vec3& operator+=(float scalar) noexcept
        {
            value += scalar;
            return *this;
        }

        constexpr vec3& operator+=(vec3 const& other) noexcept
        {
            value += other.value;
            return *this;
        }

        constexpr vec3& operator+=(glm::vec3 const& other) noexcept
        {
            value += other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Subtraction
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec3 operator-(float scalar) const noexcept
        {
            return vec3{ value - scalar };
        }

        [[nodiscard]] constexpr vec3 operator-(vec3 const& other) const noexcept
        {
            return vec3{ value - other.value };
        }

        [[nodiscard]] constexpr vec3 operator-(glm::vec3 const& other) const noexcept
        {
            return vec3{ value - other };
        }

        constexpr vec3& operator-=(float scalar) noexcept
        {
            value -= scalar;
            return *this;
        }

        constexpr vec3& operator-=(vec3 const& other) noexcept
        {
            value -= other.value;
            return *this;
        }

        constexpr vec3& operator-=(glm::vec3 const& other) noexcept
        {
            value -= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Multiplication
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec3 operator*(float scalar) const noexcept
        {
            return vec3{ value * scalar };
        }

        [[nodiscard]] constexpr vec3 operator*(vec3 const& other) const noexcept
        {
            return vec3{ value * other.value };
        }

        [[nodiscard]] constexpr vec3 operator*(glm::vec3 const& other) const noexcept
        {
            return vec3{ value * other };
        }

        constexpr vec3& operator*=(float scalar) noexcept
        {
            value *= scalar;
            return *this;
        }

        constexpr vec3& operator*=(vec3 const& other) noexcept
        {
            value *= other.value;
            return *this;
        }

        constexpr vec3& operator*=(glm::vec3 const& other) noexcept
        {
            value *= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Division
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr vec3 operator/(float scalar) const noexcept
        {
            assert(!Math::isZero(scalar));
            return vec3{ value / scalar };
        }

        [[nodiscard]] constexpr vec3 operator/(vec3 const& other) const noexcept
        {
            assert(!Math::isZero(other.value.x) && !Math::isZero(other.value.y) && !Math::isZero(other.value.z));
            return vec3{ value / other.value };
        }

        [[nodiscard]] constexpr vec3 operator/(glm::vec3 const& other) const noexcept
        {
            assert(!Math::isZero(other.x) && !Math::isZero(other.y) && !Math::isZero(other.z));
            return vec3{ value / other };
        }

        constexpr vec3& operator/=(float scalar) noexcept
        {
            assert(!Math::isZero(scalar));
            value /= scalar;
            return *this;
        }

        constexpr vec3& operator/=(vec3 const& other) noexcept
        {
            assert(!Math::isZero(other.value.x) && !Math::isZero(other.value.y) && !Math::isZero(other.value.z));
            value /= other.value;
            return *this;
        }

        constexpr vec3& operator/=(glm::vec3 const& other) noexcept
        {
            assert(!Math::isZero(other.x) && !Math::isZero(other.y) && !Math::isZero(other.z));
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

        [[nodiscard]] constexpr vec3 normalized() const noexcept
        {
            float length = glm::length(value);
            assert(!Math::isZero(length));
            return vec3(value / length);
        }

        [[nodiscard]] constexpr float dot(vec3 const& other) const noexcept
        {
            return glm::dot(value, other.value);
        }

        constexpr void setZero() noexcept
        {
            value.x = 0.0f;
            value.y = 0.0f;
            value.z = 0.0f;
        }

        [[nodiscard]] constexpr bool isZeroed() const noexcept
        {
            return Math::isZero(value.x) && Math::isZero(value.y) && Math::isZero(value.z);
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        glm::vec3& data() noexcept
        {
            return value;
        }

        glm::vec3 const& data() const noexcept
        {
            return value;
        }

        float* dataPtr() noexcept
        {
            return glm::value_ptr(value);
        }

        float const* dataPtr() const noexcept
        {
            return glm::value_ptr(value);
        }

    private:

        glm::vec3 value{ 0.0f };
    };
}

//REGISTER_TYPE_NAME(LITL::vec3)

#endif