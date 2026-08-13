#ifndef LITL_MATH_GEOMETRY_MESH_H__
#define LITL_MATH_GEOMETRY_MESH_H__

#include <type_traits>
#include <span>
#include <vector>

#include "litl-core/math/bounds/aabb.hpp"
#include "litl-core/math/geometry/vertex.hpp"

namespace litl
{
    /// <summary>
    /// A geometric mesh, which is the intermediate data representation of a full mesh object.
    /// </summary>
    struct GeoMesh
    {

        /// <summary>
        /// Returns the number of vertices in the mesh.
        /// </summary>
        [[nodiscard]] size_t vertexCount() const noexcept;

        /// <summary>
        /// Returns the number of indices in the mesh.
        /// </summary>
        [[nodiscard]] size_t indexCount() const noexcept;

        /// <summary>
        /// Returns the number of faces in the mesh.
        /// </summary>
        [[nodiscard]] size_t faceCount() const noexcept;

        /// <summary>
        /// Returns a read-only span of the mesh vertices.
        /// </summary>
        [[nodiscard]] std::span<Vertex const> getVertices() const noexcept;

        /// <summary>
        /// Returns a modifiable vector of the mesh vertices.
        /// If the vertices are modified, it is up to the caller to ensure indices and bounds also remain valid.
        /// </summary>
        std::vector<Vertex>& getVertices() noexcept;

        /// <summary>
        /// Returns a read-only span of the mesh indices.
        /// </summary>
        [[nodiscard]] std::span<uint32_t const> getIndices() const noexcept;

        /// <summary>
        /// Returns a modifiable vector of the mesh indices.
        /// If the indices are modified, it is up to the caller to ensure that vertices and face counts also remain valid.
        /// </summary>
        std::vector<uint32_t>& getIndices() noexcept;

        /// <summary>
        /// Returns a read-only span of the mesh "indices-per-face" values.
        /// </summary>
        [[nodiscard]] std::span<uint32_t const> getFaceIndexCounts() const noexcept;

        /// <summary>
        /// Returns a modifiable vector of the mesh "indices-per-face" values.
        /// If the face counts are modified, it is up to the caller to ensure that the indices also remain valid.
        /// </summary>
        /// <returns></returns>
        std::vector<uint32_t>& getFaceIndexCounts() noexcept;

        /// <summary>
        /// Sets the vertices in the mesh.
        /// If the vertices are modified, it is up to the caller to ensure indices and bounds also remain valid.
        /// </summary>
        void setVertices(std::span<Vertex const> vertices) noexcept;

        /// <summary>
        /// Sets the indices in the mesh.
        /// If the indices are modified, it is up to the caller to ensure that vertices and face counts also remain valid.
        /// </summary>
        void setIndices(std::span<uint32_t const> indices) noexcept;

        /// <summary>
        /// Sets the "indices-per-face" values for the mesh.
        /// If the face counts are modified, it is up to the caller to ensure that the indices also remain valid.
        /// </summary>
        void setFaceIndexCounts(std::span<uint32_t const> faceIndexCounts) noexcept;

        /// <summary>
        /// Sets all face index counts to the same value (for example set to 3 if all mesh faces are triangles).
        /// </summary>
        void setAllFaceIndexCounts(uint32_t count) noexcept;

        /// <summary>
        /// Returns the AABB that encapsulates the mesh.
        /// Note: this can drift out-of-sync if vertices are modified without calling recalculateBounds or setBoundsMinMax after.
        /// </summary>
        [[nodiscard]] bounds::AABB const& getBounds() const noexcept;

        /// <summary>
        /// Traverses all vertices and recalculates the bounding AABB.
        /// </summary>
        void recalculateBounds() noexcept;

        /// <summary>
        /// Sets the bounding AABB min/max points.
        /// </summary>
        void setBoundsMinMax(vec3 minPoint, vec3 maxPoint) noexcept;

        /// <summary>
        /// Recalculates and modifies the mesh in-place such that the "faceIndexCount" for every face is 3.
        /// </summary>
        void triangulate() noexcept;

    private:

        bounds::AABB m_bounds{};

        /// <summary>
        /// The distinct unique vertices within the mask. An individual vertex may be reused among multiple faces.
        /// </summary>
        std::vector<Vertex> m_vertices;

        /// <summary>
        /// The indices comprising each face of the mesh. The number of indices per face is stored in the faceIndexCount vector.
        /// </summary>
        std::vector<uint32_t> m_indices;

        /// <summary>
        /// Each index is a face, and the value is the number of vertices in the face.
        /// For example, a mesh composed of 3 triangles and 1 quad may look like [3,3,3,4].
        /// </summary>
        std::vector<uint32_t> m_faceIndexCounts;
    };
}

#endif