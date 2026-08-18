#include "litl-core/math/geometry/geoMesh.hpp"

namespace litl
{
    GeoMesh::GeoMesh()
    {

    }

    GeoMesh::~GeoMesh()
    {
        clear();
    }

    GeoMesh::GeoMesh(GeoMesh const& other)
    {
        m_bounds = other.m_bounds;
        m_vertices.assign(other.m_vertices.begin(), other.m_vertices.end());
        m_indices.assign(other.m_indices.begin(), other.m_indices.end());
        m_faceIndexCounts.assign(other.m_faceIndexCounts.begin(), other.m_faceIndexCounts.end());
    }

    GeoMesh& GeoMesh::operator=(GeoMesh const& other)
    {
        if (this != &other)
        {
            m_bounds = other.m_bounds;
            m_vertices.assign(other.m_vertices.begin(), other.m_vertices.end());
            m_indices.assign(other.m_indices.begin(), other.m_indices.end());
            m_faceIndexCounts.assign(other.m_faceIndexCounts.begin(), other.m_faceIndexCounts.end());
        }

        return *this;
    }

    GeoMesh::GeoMesh(GeoMesh&& other)
    {
        m_bounds = other.m_bounds; other.m_bounds = {};
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_faceIndexCounts = std::move(other.m_faceIndexCounts);
    }

    GeoMesh& GeoMesh::operator=(GeoMesh&& other)
    {
        if (this != &other)
        {
            m_bounds = other.m_bounds; other.m_bounds = {};
            m_vertices = std::move(other.m_vertices);
            m_indices = std::move(other.m_indices);
            m_faceIndexCounts = std::move(other.m_faceIndexCounts);
        }

        return *this;
    }

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

    void GeoMesh::setVertices(std::span<std::byte const> bytes) noexcept
    {
        std::span<Vertex const> vertices = { reinterpret_cast<Vertex const*>(bytes.data()), sizeof(Vertex) };
        setVertices(vertices);
    }

    void GeoMesh::setIndices(std::span<uint32_t const> indices) noexcept
    {
        m_indices.assign(indices.begin(), indices.end());
    }

    void GeoMesh::setIndices(std::span<std::byte const> bytes) noexcept
    {
        std::span<uint32_t const> indices = { reinterpret_cast<uint32_t const*>(bytes.data()), sizeof(uint32_t) };
        setIndices(indices);
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

    bool GeoMesh::hasNormals() const noexcept
    {
        for (auto& v : m_vertices)
        {
            if (v.normal.isZeroed())
            {
                return false;
            }
        }

        return true;
    }

    void GeoMesh::recalulateNormals(bool fast) noexcept
    {
        /**
         * To calculate normals for a mesh we must first calculate the normal for each face.
         * Then each individual vertex calculates its own normal as a weighted average of the normals
         * of the faces that it is part of. There are two primary weighting strategies:
         * 
         *     A) Weight based on triangle size. The non-normalized normals of the faces are used,
         *        and the larger the triangle then the larger this normal. The logic here is that
         *        triangles that cover more of the surface have a greater contribution to the normal.
         *        This approach is faster, but is less accurate (but typically "good enough").
         * 
         *     B) Weight based on vertex face angle size. The greater the angle, the more the vertex
         *        contributes to the overall shape of the face, and thus the contribution of the face
         *        on the vertex's normal. This is slower than (A) but provides better results.
         */

        // ... todo calculate normal of all faces. needed for both paths ...

        if (fast)
        {
            // Calculate vertex normal based on face sizes.
            // ... todo ...
        }
        else
        {
            // Calculate vertex normal based on face angle at vertex.
            // ... todo ...
        }
    }

    bool GeoMesh::isClockwiseWinding() const noexcept
    {
        // ... todo ...
        return true;
    }

    void GeoMesh::ensureClockwiseWinding() noexcept
    {
        if (!isClockwiseWinding())
        {
            return;
        }

        // ... todo ...
    }

    void GeoMesh::setWindingOrder(bool clockwise) noexcept
    {
        m_isClockwiseWinding = clockwise;
    }
}