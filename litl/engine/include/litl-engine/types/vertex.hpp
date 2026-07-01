#ifndef LITL_ENGINE_TYPES_VERTEX_H__
#define LITL_ENGINE_TYPES_VERTEX_H__

#include "litl-core/math/types.hpp"

namespace litl
{
    struct Vertex
    {
        vec3 position;
        vec3 color;
        vec2 uv;
    };
}

#endif