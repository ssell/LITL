#ifndef LITL_MATH_VEC3_H__
#define LITL_MATH_VEC3_H__

#include <cassert>
#include <format>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "litl-core/math/common.hpp"
#include "litl-core/types.hpp"

namespace litl
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
            return fequals(value.x, other.value.x) && fequals(value.y, other.value.y) && fequals(value.z, other.value.z);
        }

        [[nodiscard]] constexpr bool operator==(glm::vec3 const& other) const noexcept
        {
            return fequals(value.x, other.x) && fequals(value.y, other.y) && fequals(value.z, other.z);
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
            assert(!isZero(scalar));
            return vec3{ value / scalar };
        }

        [[nodiscard]] constexpr vec3 operator/(vec3 const& other) const noexcept
        {
            assert(!isZero(other.value.x) && !isZero(other.value.y) && !isZero(other.value.z));
            return vec3{ value / other.value };
        }

        [[nodiscard]] constexpr vec3 operator/(glm::vec3 const& other) const noexcept
        {
            assert(!isZero(other.x) && !isZero(other.y) && !isZero(other.z));
            return vec3{ value / other };
        }

        constexpr vec3& operator/=(float scalar) noexcept
        {
            assert(!isZero(scalar));
            value /= scalar;
            return *this;
        }

        constexpr vec3& operator/=(vec3 const& other) noexcept
        {
            assert(!isZero(other.value.x) && !isZero(other.value.y) && !isZero(other.value.z));
            value /= other.value;
            return *this;
        }

        constexpr vec3& operator/=(glm::vec3 const& other) noexcept
        {
            assert(!isZero(other.x) && !isZero(other.y) && !isZero(other.z));
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
        // Factory
        // ---------------------------------------------------------------------------------

        [[nodiscard]] static consteval vec3 right() noexcept
        {
            return vec3(1.0f, 0.0f, 0.0f);
        }

        [[nodiscard]] static consteval vec3 up() noexcept
        {
            return vec3(0.0f, 1.0f, 0.0f);
        }

        [[nodiscard]] static consteval vec3 forward() noexcept
        {
            return vec3(0.0f, 0.0f, 1.0f);
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
            return (value.x * value.x) + (value.y * value.y) + (value.z * value.z);
        }

        constexpr void normalize() noexcept
        {
            float length = glm::length(value);
            assert(!isZero(length));
            value = value / length;
        }

        [[nodiscard]] constexpr vec3 normalized() const noexcept
        {
            float length = glm::length(value);
            assert(!isZero(length));
            return vec3(value / length);
        }

        [[nodiscard]] constexpr float dot(vec3 const& other) const noexcept
        {
            return glm::dot(value, other.value);
        }

        [[nodiscard]] constexpr vec3 cross(vec3 const& other) const noexcept
        {
            return vec3{ glm::cross(value, other.value) };
        }

        constexpr void setZero() noexcept
        {
            value.x = 0.0f;
            value.y = 0.0f;
            value.z = 0.0f;
        }

        [[nodiscard]] constexpr bool isZeroed() const noexcept
        {
            return isZero(value.x) && isZero(value.y) && isZero(value.z);
        }

        std::string toString() const noexcept
        {
            return std::format("({:.3f},{:.3f},{:.3f})", value.x, value.y, value.z);
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr glm::vec3& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::vec3 const& data() const noexcept
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

        glm::vec3 value{ 0.0f };
    };
}

REGISTER_TYPE_NAME(litl::vec3)

#endif