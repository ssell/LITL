#include "litl-core/math/geometry/mesh.hpp"

namespace litl
{
    void GeoMesh::triangulate() noexcept
    {
        if (vertices.empty() || indices.empty() || faceIndexCount.empty())
        {
            return;
        }

        // ... todo ...
    }
}