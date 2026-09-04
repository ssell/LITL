#ifndef LITL_MATH_GEOMETRY_SUBMESH_H__
#define LITL_MATH_GEOMETRY_SUBMESH_H__

#include <cstdint>
#include "litl-core/math/bounds/aabb.hpp"

namespace litl
{
    /// <summary>
    /// A contiguous range of faces within a mesh that share a material id.
    /// </summary>
    struct Submesh
    {
        uint32_t firstIndex{ 0u };
        uint32_t indexCount{ 0u };
        uint32_t materialSlot{ 0u };
        bounds::AABB bounds{};
    };
}

#endif