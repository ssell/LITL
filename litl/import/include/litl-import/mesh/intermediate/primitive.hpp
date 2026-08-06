#ifndef LITL_IMPORT_MESH_PRIMITIVE_H__
#define LITL_IMPORT_MESH_PRIMITIVE_H__

#include <cstdint>
#include <vector>

#include "litl-core/math.hpp"

namespace litl::import
{
    enum class PrimitiveType : uint8_t
    {
        Unknown = 0u,
        Point = 1u,
        Line = 2u,
        Triangle = 3u,
        Quad = 4u
    };

    /// <summary>
    /// A single primitive in a mesh. Point, line, triangle, quad, etc.
    /// </summary>
    struct Primitive
    {
        std::vector<vec3> positions;
        std::vector<vec3> normals;
        std::vector<vec4> tangents;         // w = bitangent sign
        std::vector<vec2> uv;
        std::vector<uint32_t> indices;
    };
}

#endif