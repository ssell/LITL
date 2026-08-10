#ifndef LITL_IMPORT_MESH_H__
#define LITL_IMPORT_MESH_H__

#include <optional>
#include <vector>

#include "litl-core/math.hpp"

namespace litl::import
{
    /// <summary>
    /// Interleaves vertex, ready to be handed off to the GPU.
    /// </summary>
    struct Vertex
    {
        vec3 position{};
        vec2 texcoord{};
        vec3 normal{};
        vec4 tangent{};
    };

    /// <summary>
    /// Composed of one or more primitives.
    /// </summary>
    struct Mesh
    {
        /// <summary>
        /// Recalculates the mesh such that the "faceIndexCount" for every face is 3.
        /// </summary>
        void triangulate() noexcept;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> faceIndexCount;
    };
}

#endif