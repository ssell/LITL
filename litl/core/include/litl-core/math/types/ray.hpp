#ifndef LITL_MATH_RAY_H__
#define LITL_MATH_RAY_H__

#include "litl-core/math/types/vec3.hpp"

namespace litl
{
    struct Ray
    {
        vec3 origin{};
        vec3 direction{};
    };
}

#endif