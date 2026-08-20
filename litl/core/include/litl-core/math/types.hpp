#ifndef LITL_MATH_TYPES_H__
#define LITL_MATH_TYPES_H__

#include "litl-core/math/types/vec2.hpp"
#include "litl-core/math/types/vec3.hpp"
#include "litl-core/math/types/vec4.hpp"
#include "litl-core/math/types/mat3.hpp"
#include "litl-core/math/types/mat4.hpp"
#include "litl-core/math/types/quat.hpp"
#include "litl-core/math/types/color.hpp"
#include "litl-core/math/types/rect2D.hpp"

namespace litl
{
    /// <summary>
    /// Projects the 3D point onto a 2D plane defined by its normal.
    /// </summary>
    [[nodiscard]] constexpr vec2 project2d(vec3 planeNormal, vec3 point, vec3 origin) noexcept
    {
        if (point.isZeroed())
        {
            return vec2::zero();
        }

        const vec3 po = point - origin;
        const vec3 t = tangent(planeNormal);
        const vec3 b = cross(planeNormal, t);

        return vec2{ dot(po, t), dot(po, b) };
    }
}

#endif