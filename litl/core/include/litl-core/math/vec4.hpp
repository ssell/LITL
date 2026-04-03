#ifndef LITL_MATH_VEC4_H__
#define LITL_MATH_VEC4_H__

#include <glm/glm.hpp>

#include "litl-core/math/math.hpp"
#include "litl-core/types.hpp"

namespace LITL::Math
{
    template<typename T>
    struct Vec4Wrapper
    {
        glm::vec4 value{ 0.0f, 0.0f, 0.0f, 1.0f };

        constexpr Vec4Wrapper() = default;
        constexpr explicit Vec4Wrapper(glm::vec4 v) : value(v) {}
        constexpr Vec4Wrapper(float x, float y, float z, float w) : value(x, y, z, w) {}

        [[nodiscard]] constexpr float& x() { return value.x; }
        [[nodiscard]] constexpr float& y() { return value.y; }
        [[nodiscard]] constexpr float& z() { return value.z; }
        [[nodiscard]] constexpr float& w() { return value.w; }

        [[nodiscard]] constexpr float length() const
        {
            return glm::length(value);
        }

        void normalize() noexcept
        {
            value = glm::normalize(value);
        }

        [[nodiscard]] constexpr T normalized() const
        {
            return T{ glm::normalize(value) };
        }

        [[nodiscard]] constexpr T operator*(float s) const
        {
            return T{ value * s };
        }

        void operator*=(float s)
        {
            value *= s;
        }

        [[nodiscard]] constexpr T operator/(float s) const
        {
            return T{ value / s };
        }

        void operator/=(float s)
        {
            value /= s;
        }

        [[nodiscard]] constexpr bool operator==(Vec4Wrapper const& other) const
        {
            return 
                fequals(value.x, other.value.x) && 
                fequals(value.y, other.value.y) &&
                fequals(value.z, other.value.z) &&
                fequals(value.w, other.value.w);
        }

        [[nodiscard]] constexpr bool operator==(glm::vec4 const& other) const
        {
            return 
                fequals(value.x, other.x) && 
                fequals(value.y, other.y) &&
                fequals(value.z, other.z) &&
                fequals(value.w, other.w);
        }

        void zero() noexcept
        {
            value[0] = 0.0f;
            value[1] = 0.0f;
            value[2] = 0.0f;
            value[3] = 0.0f;
        }

        [[nodiscard]] bool isZero() const noexcept
        {
            return
                Math::isZero(value[0]) &&
                Math::isZero(value[1]) &&
                Math::isZero(value[2]) &&
                Math::isZero(value[3]);
        }

        void identity() noexcept
        {
            value[0] = 0.0f;
            value[1] = 0.0f;
            value[2] = 0.0f;
            value[3] = 1.0f;
        }

        [[nodiscard]] bool isIdentity() const noexcept
        {
            return
                Math::isZero(value[0]) &&
                Math::isZero(value[1]) &&
                Math::isZero(value[2]) &&
                Math::isOne(value[3]);
        }
    };

    struct Vec4 : Vec4Wrapper<Vec4>
    {
        using Vec4Wrapper::Vec4Wrapper;
    };
}

REGISTER_TYPE_NAME(LITL::Math::Vec4)

#endif