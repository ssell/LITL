#include "litl-core/math/geometry/geoMesh.hpp"

namespace litl
{
    void GeoMesh::clear() noexcept
    {
        m_vertices.clear();
        m_indices.clear();
        m_faceIndexCounts.clear();
    }

    size_t GeoMesh::vertexCount() const noexcept
    {
        return m_vertices.size();
    }

    size_t GeoMesh::indexCount() const noexcept
    {
        return m_indices.size();
    }

    size_t GeoMesh::faceCount() const noexcept
    {
        return m_faceIndexCounts.size();
    }

    std::span<Vertex const> GeoMesh::getVertices() const noexcept
    {
        return m_vertices;
    }

    std::vector<Vertex>& GeoMesh::getVertices() noexcept
    {
        return m_vertices;
    }

    std::span<uint32_t const> GeoMesh::getIndices() const noexcept
    {
        return m_indices;
    }

    std::vector<uint32_t>& GeoMesh::getIndices() noexcept
    {
        return m_indices;
    }

    std::span<uint32_t const> GeoMesh::getFaceIndexCounts() const noexcept
    {
        return m_faceIndexCounts;
    }

    std::vector<uint32_t>& GeoMesh::getFaceIndexCounts() noexcept
    {
        return m_faceIndexCounts;
    }

    void GeoMesh::setVertices(std::span<Vertex const> vertices) noexcept
    {
        m_vertices.assign(vertices.begin(), vertices.end());
    }

    void GeoMesh::setIndices(std::span<uint32_t const> indices) noexcept
    {
        m_indices.assign(indices.begin(), indices.end());
    }

    void GeoMesh::setFaceIndexCounts(std::span<uint32_t const> faceIndexCounts) noexcept
    {
        m_faceIndexCounts.assign(faceIndexCounts.begin(), faceIndexCounts.end());
    }

    void GeoMesh::setAllFaceIndexCounts(uint32_t count) noexcept
    {
        uint32_t const faceCount = static_cast<uint32_t>(m_indices.size()) / count;

        m_faceIndexCounts.clear();
        m_faceIndexCounts.resize(faceCount, count);
    }

    bounds::AABB const& GeoMesh::getBounds() const noexcept
    {
        return m_bounds;
    }

    void GeoMesh::recalculateBounds() noexcept
    {
        if (m_vertices.empty())
        {
            m_bounds = bounds::AABB();
            return;
        }

        vec3 minPoint = vec3::max();
        vec3 maxPoint = vec3::min();

        for (auto& vertex : m_vertices)
        {
            minPoint = min(minPoint, vertex.position);
            maxPoint = max(maxPoint, vertex.position);
        }

        setBoundsMinMax(minPoint, maxPoint);
    }

    void GeoMesh::setBoundsMinMax(vec3 minPoint, vec3 maxPoint) noexcept
    {
        m_bounds = bounds::AABB::fromMinMax(minPoint, maxPoint);
    }

    void GeoMesh::triangulate() noexcept
    {
        if (m_vertices.empty() || m_indices.empty() || m_faceIndexCounts.empty())
        {
            return;
        }

        std::vector<uint32_t> ngonFaces;
        ngonFaces.reserve(m_faceIndexCounts.size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_faceIndexCounts.size()); ++i)
        {
            if (m_faceIndexCounts[i] > 3u)
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