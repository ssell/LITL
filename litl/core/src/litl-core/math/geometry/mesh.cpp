#include "litl-core/math/geometry/mesh.hpp"

namespace litl
{
    void GeoMesh::getMinMaxPoints(vec3& minPoint, vec3& maxPoint) const noexcept
    {
        if (vertices.empty())
        {
            minPoint.setZero();
            maxPoint.setZero();
            return;
        }

        minPoint = vec3::max();
        maxPoint = vec3::min();

        for (auto& vertex : vertices)
        {
            minPoint = min(minPoint, vertex.position);
            maxPoint = max(maxPoint, vertex.position);
        }
    }

    void GeoMesh::triangulate() noexcept
    {
        if (vertices.empty() || indices.empty() || faceIndexCount.empty())
        {
            return;
        }

        std::vector<uint32_t> ngonFaces;
        ngonFaces.reserve(faceIndexCount.size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(faceIndexCount.size()); ++i)
        {
            if (faceIndexCount[i] > 3u)
            {
                ngonFaces.push_back(i);
            }
        }

        if (ngonFaces.empty())
        {
            return;
        }

        // ... todo not yet needed for current test models but will need in the future ...
        // ... use ear-clipping as i did long ago, but need to project to a 2D plane based on face normal ...
    }
}