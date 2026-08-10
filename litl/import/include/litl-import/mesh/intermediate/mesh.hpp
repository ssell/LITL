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
        /// Adds the indices for a single triangle;
        /// </summary>
        void addTriangle(uint32_t index0, uint32_t index1, uint32_t index2) noexcept;

        /// <summary>
        /// Adds the indices for a single quad.
        /// </summary>
        void addQuad(uint32_t index0, uint32_t index1, uint32_t index2, uint32_t index3) noexcept;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };
}

#endif