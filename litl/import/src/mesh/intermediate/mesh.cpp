#include "litl-import/mesh/intermediate/mesh.hpp"

namespace litl::import
{
    void Mesh::reserveAttributesCount(uint32_t count) noexcept
    {
        positions.reserve(count);
        normals.reserve(count);
        uvs.reserve(count);
        tangents.reserve(count);
    }

    void Mesh::reserveIndexCount(uint32_t count) noexcept
    {
        indices.reserve(count);
    }

    void Mesh::addVertex(vec3 position, vec2 uv, vec3 normal, vec4 tangent) noexcept
    {
        positions.push_back(position);
        uvs.push_back(uv);
        normals.push_back(normal);
        tangents.push_back(tangent);
    }

    void Mesh::addTriangle(uint32_t index0, uint32_t index1, uint32_t index2) noexcept
    {
        indices.push_back(index0);
        indices.push_back(index1);
        indices.push_back(index2);
    }

    void Mesh::addQuad(uint32_t index0, uint32_t index1, uint32_t index2, uint32_t index3) noexcept
    {
        // Subdivide the quad into two triangles.
        indices.push_back(index0);
        indices.push_back(index1);
        indices.push_back(index2);

        indices.push_back(index2);
        indices.push_back(index3);
        indices.push_back(index0);
    }
}