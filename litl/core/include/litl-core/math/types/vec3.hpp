#ifndef LITL_MATH_VEC3_H__
#define LITL_MATH_VEC3_H__

#include <cassert>
#include <format>
#include <limits>
#include <string>

#include "litl-core/math/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "litl-core/math/common.hpp"
#include "litl-core/types.hpp"

namespace litl
{
    struct vec4;

    struct vec3
    {
        constexpr vec3() {}
        constexpr vec3(vec3 const& other) : value{ other.value } {}
        constexpr explicit vec3(float xyz) : value{ xyz, xyz, xyz } {}
        constexpr explicit vec3(glm::vec3 const& other) : value{other} {}
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

        /// <summary>
        /// (1, 0, 0)
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static constexpr vec3 right() noexcept
        {
            return vec3(1.0f, 0.0f, 0.0f);
        }

        /// <summary>
        /// (0, 1, 0)
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static constexpr vec3 up() noexcept
        {
            return vec3(0.0f, 1.0f, 0.0f);
        }

        /// <summary>
        /// (0, 0, 1)
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static constexpr vec3 forward() noexcept
        {
            return vec3(0.0f, 0.0f, 1.0f);
        }

        [[nodiscard]] static constexpr vec3 min() noexcept
        {
            return vec3{ std::numeric_limits<float>::min() };
        }

        [[nodiscard]] static constexpr vec3 max() noexcept
        {
            return vec3{ std::numeric_limits<float>::max() };
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

        [[nodiscard]] constexpr float distanceTo(vec3 other) const noexcept
        {
            return glm::distance(value, other.value);
        }

        [[nodiscard]] constexpr float distanceSqTo(vec3 other) const noexcept
        {
            glm::vec3 diff = value - other.value;
            return glm::dot(diff, diff);
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

        /// <summary>
        /// The dot product is the measure of how aligned two vectors are.
        /// Though the result differs depending on if both, one, or none of the
        /// vectors are normalized.<br/>
        /// 
        /// If both vectors are normalized, then the dot product is on
        /// the range of [-1, 1] and provides pure directional similarity.
        /// Where 1 = same direction, 0 = orthogonal, -1 = opposite directions.<br/>
        /// 
        /// If one vector is normalized, then the dot product is the
        /// scalar projection of the non-normalized vector onto the normalized
        /// directional vector. The result is "how much of the non-normalized
        /// vector lies on the normalized directional vector?"<br/>
        /// 
        /// If neither vector is normalized then the result mixes together
        /// both the direction and scale. Large values can mean either that
        /// there is a strong alignment, there are large magnitudes, or both.<br/>
        /// 
        /// And no, order does not matter for dot product (but it does for cross).
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr float dot(vec3 const& other) const noexcept
        {
            return glm::dot(value, other.value);
        }

        /// <summary>
        /// Given two vectors, they can be interpreted to define a finite patch
        /// of a plane in the form of a parallelogram. The order of the vectors
        /// in the cross product operation ((a x b) vs (b x a)) determines the 
        /// normal direction of the plane (flips).<br/>
        /// 
        /// In all cases (two, one, no normalized input vectors) the result of
        /// cross product is an encoded vector representing the parallelogram.<br/>
        /// 
        /// If both vectors are normalized (‖a‖ = ‖b‖ = 1), then: ‖a x b‖ = sinθ.
        /// The normalized form of the vector is the normal of the plane which is a 
        /// third vector that is orthogonal to the two input vectors. The magnitude is a measure 
        /// of "how perpendicular" the vectors are two each other, which lies on the range 
        /// of [0, 1] where 0 = the vectors are the same, and where 1 = the vectors are completely 
        /// perpendicular to each other (θ = π/2).<br/>
        /// 
        /// If one vector is normalized (‖b‖ = 1), then: ‖a x b‖ = ‖a‖ sinθ.
        /// The resulting vector is still the orthogonal directional vector when normalized.
        /// However, if not normalized the magnitude is the length of the component of a perpendicular to b.
        /// So "how wide" the first vector spreads perpendicular to the second.<br/>
        /// 
        /// If neither vector is normalized: a x b = (‖a‖ ‖b‖ sinθ)n
        /// Once again, the normalized direction is the normal of the plane. However, this time
        /// the magnitude is the area of the parallelogram. This case is used in calculating surface normals.
        /// </summary>
        /// <param name="other"></param>
        /// <returns></returns>
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

    /// <summary>
    /// Returns a vector where each component value is the minimum value of the same component in the two provided vectors.
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr vec3 min(vec3 a, vec3 b) noexcept
    {
        return vec3{ glm::min(a.data(), b.data()) };
    }

    /// <summary>
    /// Returns a vector where each component value is the maximum value of the same component in the two provided vectors.
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr vec3 max(vec3 a, vec3 b) noexcept
    {
        return vec3{ glm::max(a.data(), b.data()) };
    }

    /// <summary>
    /// See vec3::dot
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr float dot(vec3 a, vec3 b) noexcept
    {
        return a.dot(b);
    }

    /// <summary>
    /// See vec3::cross
    /// </summary>
    /// <param name="a"></param>
    /// <param name="b"></param>
    /// <returns></returns>
    [[nodiscard]] constexpr vec3 cross(vec3 a, vec3 b) noexcept
    {
        return a.cross(b);
    }

    [[nodiscard]] constexpr vec3 midpoint(vec3 a, vec3 b) noexcept
    {
        return (a + (b - a) * 0.5f);
    }

    [[nodiscard]] constexpr vec3 normalize(vec3 a) noexcept
    {
        return a.normalized();
    }

    [[nodiscard]] constexpr vec3 clamp(vec3 v, vec3 min, vec3 max) noexcept
    {
        return vec3{ glm::clamp(v.data(), min.data(), max.data()) };
    }

    [[nodiscard]] constexpr float distance(vec3 a, vec3 b) noexcept
    {
        return a.distanceTo(b);
    }

    [[nodiscard]] constexpr float distanceSq(vec3 a, vec3 b) noexcept
    {
        return a.distanceSqTo(b);
    }

    [[nodiscard]] constexpr vec3 abs(vec3 v) noexcept
    {
        return vec3{ glm::abs(v.data()) };
    }
}

REGISTER_TYPE_NAME(litl::vec3)

#endif