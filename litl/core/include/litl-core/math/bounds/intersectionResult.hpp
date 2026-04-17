#ifndef LITL_MATH_BOUNDS_INTERSECTION_RESULT_H__
#define LITL_MATH_BOUNDS_INTERSECTION_RESULT_H__

#include "litl-core/math/types/vec3.hpp"

namespace litl::bounds
{
    struct IntersectionResult
    {
        bool intersects{ false };
        vec3 point{};
    };
}

#endif