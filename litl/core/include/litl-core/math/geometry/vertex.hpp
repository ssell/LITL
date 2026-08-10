#ifndef LITL_CORE_MATH_GEOMETRY_VERTEX_H__
#define LITL_CORE_MATH_GEOMETRY_VERTEX_H__

#include "litl-core/math/types.hpp"

namespace litl
{
    struct Vertex
    {
        vec3 position{};
        vec2 texcoord{};
        vec3 normal{};
        vec4 tangent{};
    };
}

#endif