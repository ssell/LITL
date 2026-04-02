#ifndef LITL_MATH_VEC3_H__
#define LITL_MATH_VEC3_H__

#include <glm/glm.hpp>

#include "litl-core/math/math.hpp"
#include "litl-core/types.hpp"

namespace LITL::Math
{
    template<typename T>
    struct Vec3Wrapper
    {
        glm::vec3 value{ 0.0f };

        constexpr Vec3Wrapper() = default;
        constexpr explicit Vec3Wrapper(glm::vec3 v) : value(v) {}
        constexpr Vec3Wrapper(float x, float y, float z) : value(x, y, z) {}

        constexpr float& x() { return value.x; }
        constexpr float& y() { return value.y; }
        constexpr float& z() { return value.z; }

        constexpr float length() const
        {
            return glm::length(value);
        }

        constexpr T normalized() const
        {
            return T{ glm::normalize(value) };
        }

        constexpr T operator*(float s) const
        {
            return T{ value * s };
        }

        void operator*=(float s)
        {
            value *= s;
        }

        constexpr T operator/(float s) const
        {
            return T{ value / s };
        }

        void operator/=(float s)
        {
            value /= s;
        }

        constexpr bool operator==(Vec3Wrapper const& other)
        {
            return fequals(value.x, other.value.x) && fequals(value.y, other.value.y) && fequals(value.z, other.value.z);
        }

        constexpr bool operator==(glm::vec3 const& other)
        {
            return fequals(value.x, other.x) && fequals(value.y, other.y) && fequals(value.z, other.z);
        }
    };

    struct Vec3 : Vec3Wrapper<Vec3>
    {
        using Vec3Wrapper::Vec3Wrapper;
    };
}

REGISTER_TYPE_NAME(LITL::Math::Vec3)

#endif