#include "litl-import/mesh/intermediate/mesh.hpp"

namespace litl::import
{
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