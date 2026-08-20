#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-core/math/geometry/tools/normals.hpp"
#include "litl-core/math/geometry/tools/orientation.hpp"

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
        calculateMeshNormals(m_vertices, m_indices);
    }

    bool GeoMesh::hasTexcoords() const noexcept
    {
        for (auto& v : m_vertices)
        {
            // Consider both external origin points (0,0) and vulkan origin points (0,1) 
            if (!isZero(v.texcoord.x()) || (between(v.texcoord.y(), Traits<float>::epsilon, 1.0f - Traits<float>::epsilon)))
            {
                return true;
            }
        }

        return false;
    }

    MeshWinding GeoMesh::getWinding() const noexcept
    {
        return m_winding;
    }

    void GeoMesh::ensureClockwiseWinding() noexcept
    {
        if (m_winding == MeshWinding::Clockwise)
        {
            return;
        }

        if (m_winding == MeshWinding::Unknown)
        {
            MeshOrientationReport orientationReport = orientateMesh(m_vertices, m_indices);

            if (orientationReport.nonOrientable)
            {
                logWarning("Called to ensure winding on a non-orientable mesh.");
            }
            else
            {
                setWindingOrder(MeshWinding::Clockwise);
            }
        }
        else
        {
            const uint32_t faceCount = m_indices.size() / 3u;

            if (faceCount > 0u)
            {
                for (uint32_t face = 0u; face < faceCount; ++face)
                {
                    std::swap(m_indices[(face * 3) + 1], m_indices[(face * 3) + 2]);
                }
            }
        }
    }

    void GeoMesh::setWindingOrder(MeshWinding winding) noexcept
    {
        m_winding = winding;
    }

    void GeoMesh::negateZValues() noexcept
    {
        for (auto& vertex : m_vertices)
        {
            vertex.position.z() = -vertex.position.z();
            vertex.normal.z() = -vertex.normal.z();
        }
    }

    void GeoMesh::flipTexcoordV() noexcept
    {
        for (auto& vertex : m_vertices)
        {
            vertex.texcoord.y() = 1.0f - clamp(vertex.texcoord.y(), 0.0f, 1.0f);
        }
    }
}