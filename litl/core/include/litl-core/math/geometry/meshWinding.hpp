#ifndef LITL_CORE_MATH_GEOMETRY_MESH_WINDING_H__
#define LITL_CORE_MATH_GEOMETRY_MESH_WINDING_H__

#include <cstdint>

namespace litl
{
    enum class MeshWinding : uint32_t
    {
        /// <summary>
        /// The winding has not been calculated or set.
        /// </summary>
        Unknown = 0u,

        /// <summary>
        /// Winding is clockwise which is what LITL uses internally due to its left-handed coordinate system.
        /// </summary>
        Clockwise = 1u,

        /// <summary>
        /// Winding is counter-clockwise which is typical for externally authored meshes.
        /// </summary>
        CounterClockwise = 2u
    };
}

#endif