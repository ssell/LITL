#ifndef LITL_MATH_GEOMETRY_SUBMESH_H__
#define LITL_MATH_GEOMETRY_SUBMESH_H__

#include <array>
#include <cstdint>
#include <type_traits>

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
        std::array<uint32_t, 3> padding{};
    };

    static_assert(sizeof(Submesh) == 48);
    static_assert((sizeof(Submesh) % 16) == 0);
    static_assert(std::is_trivially_copyable_v<Submesh>);
    static_assert(std::is_standard_layout_v<Submesh>);
}

#endif