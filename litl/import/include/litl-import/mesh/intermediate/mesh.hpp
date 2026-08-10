#ifndef LITL_IMPORT_MESH_H__
#define LITL_IMPORT_MESH_H__

#include <vector>

#include "litl-core/math.hpp"

namespace litl::import
{
    /// <summary>
    /// Composed of one or more primitives.
    /// </summary>
    struct Mesh
    {
        /// <summary>
        /// Reserves memory for the specified number of elements for each attribute.
        /// </summary>
        void reserveAttributesCount(uint32_t count) noexcept;

        /// <summary>
        /// Reserves memory for the specified number of indices.
        /// </summary>
        void reserveIndexCount(uint32_t count) noexcept;

        /// <summary>
        /// Adds the attributes for a single vertex.
        /// </summary>
        void addVertex(vec3 position, vec2 uv, vec3 normal, vec4 tangent) noexcept;

        /// <summary>
        /// Adds the indices for a single triangle;
        /// </summary>
        void addTriangle(uint32_t index0, uint32_t index1, uint32_t index2) noexcept;

        /// <summary>
        /// Adds the indices for a single quad.
        /// </summary>
        void addQuad(uint32_t index0, uint32_t index1, uint32_t index2, uint32_t index3) noexcept;

        std::vector<vec3> positions;
        std::vector<vec2> uvs;
        std::vector<vec3> normals;
        std::vector<vec4> tangents;
        std::vector<uint32_t> indices;
    };
}

#endif