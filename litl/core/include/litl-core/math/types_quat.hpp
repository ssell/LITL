#ifndef LITL_MATH_QUAT_H__
#define LITL_MATH_QUAT_H__

#include <cassert>
#include <format>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "litl-core/math/common.hpp"
#include "litl-core/types.hpp"
#include "litl-core/math/types_vec3.hpp"

namespace LITL
{
    struct mat3;
    struct mat4;

    struct quat
    {
        constexpr quat() {}
        constexpr quat(quat const& other) : value(other.value) {}
        constexpr explicit quat(glm::quat const& other) : value(other) {}
        constexpr quat(float w, float x, float y, float z) : value(w, x, y, z) {}
        explicit quat(mat3 const& matrix);
        explicit quat(mat4 const& matrix);

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(quat const& other) const noexcept
        {
            return Math::fequals(value.x, other.value.x) && Math::fequals(value.y, other.value.y) && Math::fequals(value.z, other.value.z) && Math::fequals(value.w, other.value.w);
        }

        [[nodiscard]] constexpr bool operator==(glm::quat const& other) const noexcept
        {
            return Math::fequals(value.x, other.x) && Math::fequals(value.y, other.y) && Math::fequals(value.z, other.z) && Math::fequals(value.w, other.w);
        }

        // ---------------------------------------------------------------------------------
        // Negation
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr quat operator-() const noexcept
        {
            return quat{ -value };
        }

        // ---------------------------------------------------------------------------------
        // Multiplication
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr quat operator*(quat const& other) const noexcept
        {
            return quat{ value * other.value };
        }

        [[nodiscard]] vec3 operator*(vec3 const& v) const noexcept
        {
            return vec3{ value * v.data() };
        }

        [[nodiscard]] quat operator*(float scalar) const noexcept
        {
            return quat{ value * scalar };
        }

        constexpr quat& operator*=(quat const& other) noexcept
        {
            value = value * other.value;
            return *this;
        }

        constexpr quat& operator*=(float scalar) noexcept
        {
            value = value * scalar;
            return *this;
        }

        [[nodiscard]] vec3 rotate(vec3 const& v) const noexcept
        {
            return (*this * v);
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
        // Factory
        // ---------------------------------------------------------------------------------

        [[nodiscard]] static constexpr quat fromAxisAngle(vec3 const& axis, float angle) noexcept
        {
            return quat{ glm::angleAxis(angle, axis.data()) };
        }

        [[nodiscard]] static constexpr quat fromEuler(float yaw, float pitch, float roll) noexcept
        {
            return quat{ glm::vec3{ pitch, yaw, roll } };
        }

        [[nodiscard]] static constexpr quat lookRotation(vec3 const& forward, vec3 const& up) noexcept
        {
            return quat{ glm::quatLookAt(forward.data(), up.data()) };
        }

        [[nodiscard]] static constexpr quat identity() noexcept
        {
            return quat{ 1.0f, 0.0f, 0.0f, 0.0f };
        }

        [[nodiscard]] static constexpr quat fromVector(vec3 const& from, vec3 const& to) noexcept
        {
            return quat{ glm::rotation(from.data(), to.data()) };
        }

        [[nodiscard]] static constexpr quat fromAngleX(float angle) noexcept
        {
            return quat{ glm::angleAxis(angle, glm::vec3{ 1.0f, 0.0f, 0.0f }) };
        }

        [[nodiscard]] static constexpr quat fromAngleY(float angle) noexcept
        {
            return quat{ glm::angleAxis(angle, glm::vec3{ 0.0f, 1.0f, 0.0f }) };
        }

        [[nodiscard]] static constexpr quat fromAngleZ(float angle) noexcept
        {
            return quat{ glm::angleAxis(angle, glm::vec3{ 0.0f, 0.0f, 1.0f }) };
        }

        // ---------------------------------------------------------------------------------
        // Utility
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool isIdentity() const noexcept
        {
            return Math::isOne(value.w) && Math::isZero(value.x) && Math::isZero(value.y) && Math::isZero(value.z);
        }

        [[nodiscard]] constexpr quat getInverse() const noexcept
        {
            return quat{ glm::inverse(value) };
        }

        constexpr void inverse() noexcept
        {
            value = glm::inverse(value);
        }

        [[nodiscard]] constexpr quat getConjugate() const noexcept
        {
            return quat{ glm::conjugate(value) };
        }

        constexpr void conjugate() noexcept
        {
            value = glm::conjugate(value);
        }

        [[nodiscard]] constexpr quat getNormalized() const noexcept
        {
            return quat{ glm::normalize(value) };
        }

        constexpr void normalize() noexcept
        {
            value = { 1.0f, 0.0f, 0.0f, 0.0f };
        }

        [[nodiscard]] constexpr float length() const noexcept
        {
            return glm::length(value);
        }

        [[nodiscard]] constexpr float lengthSquared() const noexcept
        {
            return (value.x * value.x) + (value.y * value.y) + (value.z * value.z) + (value.w * value.w);
        }

        [[nodiscard]] constexpr float dot(quat const& other) const noexcept
        {
            return glm::dot(value, other.value);
        }

        [[nodiscard]] constexpr quat lerp(quat const& other, float t) const noexcept
        {
            return quat{ glm::lerp(value, other.value, t) };
        }

        [[nodiscard]] constexpr quat slerp(quat const& other, float t) const noexcept
        {
            return quat{ glm::slerp(value, other.value, t) };
        }

        [[nodiscard]] constexpr vec3 axis() const noexcept
        {
            return vec3{ glm::axis(value) };
        }

        [[nodiscard]] constexpr float angle() const noexcept
        {
            return glm::angle(value);
        }

        [[nodiscard]] constexpr vec3 forward() const noexcept
        {
            return vec3{ glm::normalize(value * glm::vec3(0.0f, 0.0f, -1.0f)) };
        }

        [[nodiscard]] constexpr vec3 right() const noexcept
        {
            return vec3{ glm::normalize(value * glm::vec3(1.0f, 0.0f, 0.0f)) };
        }

        [[nodiscard]] constexpr vec3 up() const noexcept
        {
            return vec3{ glm::normalize(value * glm::vec3(0.0f, 1.0f, 0.0f)) };
        }

        [[nodiscard]] constexpr float angleBetween(quat const& other) const noexcept
        {
            float cosTheta = dot(other);
            return 2.0f * glm::acos(glm::min(glm::abs(cosTheta), 1.0f));
        }

        [[nodiscard]] mat3 toMat3() const noexcept;
        [[nodiscard]] mat4 toMat4() const noexcept;

        [[nodiscard]] vec3 toEuler() const noexcept
        {
            return vec3{ glm::yaw(value), glm::pitch(value), glm::roll(value) };
        }

        [[nodiscard]] float yaw() const noexcept
        {
            return glm::yaw(value);
        }

        [[nodiscard]] float pitch() const noexcept
        {
            return glm::pitch(value);
        }

        [[nodiscard]] float roll() const noexcept
        {
            return glm::roll(value);
        }

        std::string toString() const noexcept
        {
            return std::format("({:.3f},{:.3f},{:.3f},{:.3f})", value.w, value.x, value.y, value.z);
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr glm::quat& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::quat const& data() const noexcept
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

        glm::quat value{ 1.0f, 0.0f, 0.0f, 0.0f };
    };
}

REGISTER_TYPE_NAME(LITL::quat)

#endif