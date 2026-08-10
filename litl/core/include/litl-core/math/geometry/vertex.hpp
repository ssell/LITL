#ifndef LITL_CORE_MATH_GEOMETRY_VERTEX_H__
#define LITL_CORE_MATH_GEOMETRY_VERTEX_H__

#include "litl-core/math/types.hpp"

namespace litl
{
    /// <summary>
    /// The core LITL vertex representation.
    /// 
    /// Other vertex layouts may be used, but this is the structure that is defaulted to
    /// and that imported meshes are built upon.
    /// </summary>
    struct Vertex
    {
        vec3 position{};
        vec2 texcoord{};
        vec3 normal{};
        vec4 tangent{};
    };
}

#endif