#include <algorithm>

#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-core/math/geometry/tools/normals.hpp"
#include "litl-core/math/geometry/tools/orientation.hpp"
#include "litl-core/math/geometry/tools/triangulate.hpp"

namespace litl
{
    void GeoMesh::clear() noexcept
    {
        m_vertices.clear();
        m_indices.clear();
        m_faceIndexCounts.clear();
        m_faceMaterialSlots.clear();
        m_submeshes.clear();
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

    size_t GeoMesh::submeshCount() const noexcept
    {
        return m_submeshes.size();
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

    std::span<uint32_t const> GeoMesh::getFaceMaterialSlots() const noexcept
    {
        return m_faceMaterialSlots;
    }

    std::vector<uint32_t>& GeoMesh::getFaceMaterialSlots() noexcept
    {
        return m_faceMaterialSlots;
    }

    std::span<Submesh const> GeoMesh::getSubmeshes() const noexcept
    {
        return m_submeshes;
    }

    std::vector<Submesh>& GeoMesh::getSubmeshes() noexcept
    {
        return m_submeshes;
    }

    void GeoMesh::setVertices(std::span<Vertex const> vertices) noexcept
    {
        m_vertices.assign(vertices.begin(), vertices.end());
    }

    void GeoMesh::setVertices(std::span<std::byte const> bytes) noexcept
    {
        std::span<Vertex const> vertices = { reinterpret_cast<Vertex const*>(bytes.data()), bytes.size() / sizeof(Vertex) };
        setVertices(vertices);
    }

    void GeoMesh::setIndices(std::span<uint32_t const> indices) noexcept
    {
        m_indices.assign(indices.begin(), indices.end());
    }

    void GeoMesh::setIndices(std::span<std::byte const> bytes) noexcept
    {
        std::span<uint32_t const> indices = { reinterpret_cast<uint32_t const*>(bytes.data()), bytes.size() / sizeof(uint32_t) };
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

    void GeoMesh::setFaceDefaultMaterialSlots() noexcept
    {
        m_faceMaterialSlots.clear();
        m_faceMaterialSlots.resize(m_faceIndexCounts.size(), Constants::uint32_null_index);
    }

    void GeoMesh::setSubmeshes(std::span<Submesh const> submeshes) noexcept
    {
        m_submeshes.assign(submeshes.begin(), submeshes.end());
    }

    void GeoMesh::setSubmeshes(std::span<std::byte const> bytes) noexcept
    {
        std::span<Submesh const> submeshes = { reinterpret_cast<Submesh const*>(bytes.data()), bytes.size() / sizeof(Submesh) };
        setSubmeshes(submeshes);
    }

    bounds::AABB const& GeoMesh::getBounds() const noexcept
    {
        return m_bounds;
    }

    std::optional<bounds::AABB> GeoMesh::getSubmeshBounds(uint32_t submeshIndex) const noexcept
    {
        if (submeshIndex >= m_submeshes.size())
        {
            return std::nullopt;
        }

        return m_submeshes[submeshIndex].bounds;
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

    bool GeoMesh::recalculateSubmeshBounds(uint32_t submeshIndex, ErrorCode& error) noexcept
    {
        if (submeshIndex >= m_submeshes.size())
        {
            error = ErrorCode::InvalidSubmeshIndex;
            return false;
        }

        auto& submesh = m_submeshes[submeshIndex];

        if ((submesh.firstIndex + submesh.indexCount) > m_indices.size())
        {
            error = ErrorCode::InvalidSubmeshRange;
            return false;
        }

        vec3 minPoint = vec3::max();
        vec3 maxPoint = vec3::min();

        for (uint32_t index = submesh.firstIndex; index < (submesh.firstIndex + submesh.indexCount); ++index)
        {
            minPoint = min(minPoint, m_vertices[m_indices[index]].position);
            maxPoint = max(maxPoint, m_vertices[m_indices[index]].position);
        }

        submesh.bounds = bounds::AABB::fromMinMax(minPoint, maxPoint);

        return true;
    }

    void GeoMesh::setBoundsMinMax(vec3 minPoint, vec3 maxPoint) noexcept
    {
        m_bounds = bounds::AABB::fromMinMax(minPoint, maxPoint);
    }

    MeshTriangulationReport GeoMesh::triangulate() noexcept
    {
        if (m_vertices.empty() || m_indices.empty() || m_faceIndexCounts.empty())
        {
            return {};
        }

        std::vector<uint32_t> triangulatedIndices;
        std::vector<uint32_t> triangulatedFaceMaterialSlots;

        const auto report = triangulateMesh(m_vertices, m_indices, m_faceIndexCounts, m_faceMaterialSlots, triangulatedIndices, triangulatedFaceMaterialSlots);

        if (report.success)
        {
            m_indices = std::move(triangulatedIndices);
            m_faceMaterialSlots = std::move(triangulatedFaceMaterialSlots);
            setAllFaceIndexCounts(3u);
        }

        return report;
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

                setWindingOrder(MeshWinding::Clockwise);
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

    bool GeoMesh::validateSubmeshes(std::span<Submesh const> submeshes, uint32_t indexCount, ErrorCode& error) noexcept
    {
        if (submeshes.empty())
        {
            error = ErrorCode::MissingSubmesh;
            return false;
        }

        uint64_t cursor = 0ull;

        for (auto& submesh : submeshes)
        {
            if (submesh.indexCount == 0u)
            {
                error = ErrorCode::InvalidSubmeshRange;
                return false;
            }

            if (submesh.firstIndex != cursor)
            {
                error = ErrorCode::InvalidSubmeshCoverage;
                return false;
            }

            cursor += submesh.indexCount;

            if (cursor > indexCount)
            {
                error = ErrorCode::InvalidSubmeshRange;
                return false;
            }
        }

        if (cursor != indexCount)
        {
            error = ErrorCode::InvalidSubmeshCoverage;
            return false;
        }

        return true;
    }

    bool GeoMesh::validateSubmeshes(ErrorCode& error) const noexcept
    {
        return validateSubmeshes(m_submeshes, indexCount(), error);
    }

    bool GeoMesh::finalizeSubmeshes(ErrorCode& error) noexcept
    {
        if (m_submeshes.empty())
        {
            m_submeshes.push_back(Submesh{
                .firstIndex = 0u,
                .indexCount = static_cast<uint32_t>(m_indices.size()),
                .materialSlot = Constants::uint32_null_index,
                .bounds = m_bounds,
            });
        }
        else
        {
            std::sort(m_submeshes.begin(), m_submeshes.end(), [](Submesh const& a, Submesh const& b) -> bool
            {
                return (a.firstIndex < b.firstIndex);
            });

            for (uint32_t i = 0u; i < static_cast<uint32_t>(m_submeshes.size()); ++i)
            {
                if (!recalculateSubmeshBounds(i, error))
                {
                    return false;
                }
            }
        }

        if (!validateSubmeshes(error))
        {
            return false;
        }

        return true;
    }
}