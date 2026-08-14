#ifndef LITL_ENGINE_OBJECTS_MESH_H__
#define LITL_ENGINE_OBJECTS_MESH_H__

#include <memory>
#include <span>

#include "litl-core/authority.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/math/geometry/geoMesh.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/mesh.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    class ObjectPool;

    struct MeshVertexDescriptor
    {
        /// <summary>
        /// The number of vertices in the mesh.
        /// </summary>
        uint32_t vertexCount{ 0u };

        /// <summary>
        /// The number of bytes in a single vertex.
        /// </summary>
        uint32_t vertexByteSize{ 0u };

        /// <summary>
        /// If vertex data is supplied at time of mesh construction, then this is the data.
        /// </summary>
        std::span<std::byte const> vertexData;
    };

    struct MeshIndexDescriptor
    {
        /// <summary>
        /// The number of indices in the mesh.
        /// </summary>
        uint32_t indexCount{ 0u };

        /// <summary>
        /// The number of bytes in a single index.
        /// </summary>
        uint32_t indexByteSize{ 0u };

        /// <summary>
        /// If index data is supplied at time of mesh construction, then this is the data.
        /// </summary>
        std::span<std::byte const> indexData;
    };

    struct MeshDescriptor
    {
        ObjectDescriptor objectInfo{};
        MeshVertexDescriptor vertexInfo{};
        MeshIndexDescriptor indexInfo{};
    };

    /// <summary>
    /// Combination of a vertex and index buffer.
    /// Note: for early development simplicity, meshes are currently not modifiable after they are initially created.
    /// </summary>
    class Mesh
    {
    public:

        enum class ErrorCode : uint32_t
        {
            None = 0u,
            VertexBufferCreationFailed = 1u,
            IndexBufferCreationFailed = 2u,
            InvalidVertexElementSize = 3u,
            InvalidVertexElementSizeForCpu = 4u,
            InvalidIndexElementSize = 5u,
            InvalidIndexElementSizeForCpu = 6u,
            EmptyVertexDataSource = 7u,
            EmptyIndexDataSource = 8u
        };

        static constexpr std::array<std::string_view, 9> ErrorStrings{
            "None",
            "Vertex Buffer Creation Failed",
            "Index Buffer Creation Failed",
            "Invalid Vertex Element Size",
            "Invalid Vertex Element Size For Cpu",
            "Invalid Index Element Size",
            "Invalid Index Element Size For Cpu",
            "Empty Vertex Data Source",
            "Empty Index Data Source"
        };

        /// <summary>
        /// Path when being created all at once.
        /// </summary>
        [[nodiscard]] bool create(Authority<ObjectPool> auth, ObjectPool& pool, MeshDescriptor const& descriptor, ErrorCode& error) noexcept;

        /// <summary>
        /// Path when being created incrementally by the asset system.
        /// </summary>
        [[nodiscard]] bool create(Authority<ObjectPool> auth, ObjectPool& pool, ObjectDescriptor const& descriptor, ErrorCode& error) noexcept;

        /// <summary>
        /// Destroys both the CPU and GPU copies of the underlying buffers.
        /// </summary>
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// Retrieves the descriptor that was used to create this mesh.
        /// </summary>
        [[nodiscard]] MeshDescriptor const& getDescriptor() const noexcept;

        /// <summary>
        /// Retrieves the handle of the underlying vertex buffer.
        /// </summary>
        [[nodiscard]] GpuBufferHandle getVertexBuffer() const noexcept;

        /// <summary>
        /// Retrieves the handle of the underlying index buffer.
        /// </summary>
        [[nodiscard]] GpuBufferHandle getIndexBuffer() const noexcept;

        /// <summary>
        /// Sets the vertices for the mesh.
        /// </summary>
        /// <param name="toCpu">If true, the data is copied into the internal GeoMesh.</param>
        /// <param name="toGpu">If true, the data is copied into the internal GpuBuffer.</param>
        template<typename T> requires std::is_trivially_copyable_v<T>
        [[nodiscard]] bool setVertices(std::span<T const> data, bool toCpu, bool toGpu, ErrorCode& error) noexcept
        {
            return setVertices(as_byte_span(data), sizeof(T), toCpu, toGpu, error);
        }

        /// <summary>
        /// Sets the vertices for the mesh.
        /// </summary>
        /// <param name="toCpu">If true, the data is copied into the internal GeoMesh.</param>
        /// <param name="toGpu">If true, the data is copied into the internal GpuBuffer.</param>
        [[nodiscard]] bool setVertices(std::span<std::byte const> data, size_t vertexElementSize, bool toCpu, bool toGpu, ErrorCode& error) noexcept;

        /// <summary>
        /// Sets the indices for the mesh.
        /// </summary>
        /// <param name="toCpu">If true, the data is copied into the internal GeoMesh.</param>
        /// <param name="toGpu">If true, the data is copied into the internal GpuBuffer.</param>
        template<typename T> requires std::is_trivially_copyable_v<T>
        [[nodiscard]] bool setIndices(std::span<T const> data, bool toCpu, bool toGpu, ErrorCode& error) noexcept
        {
            return setIndices(as_byte_span(data), sizeof(T), toCpu, toGpu, error);
        }

        /// <summary>
        /// Sets the indices for the mesh.
        /// </summary>
        /// <param name="toCpu">If true, the data is copied into the internal GeoMesh.</param>
        /// <param name="toGpu">If true, the data is copied into the internal GpuBuffer.</param>
        [[nodiscard]] bool setIndices(std::span<std::byte const> data, size_t indexElementSize, bool toCpu, bool toGpu, ErrorCode& error) noexcept;

        /// <summary>
        /// Attempts to upload any changes to the underlying GeoMesh to the GPU.
        /// </summary>
        [[nodiscard]] bool uploadCpuMeshToGpu(ErrorCode& error) noexcept;

        /// <summary>
        /// Retrieves the underlying CPU-side GeoMesh which may or may not be in memory still.
        /// </summary>
        [[nodiscard]] GeoMesh& getGeoMesh() noexcept;

        /// <summary>
        /// Retrieves the underlying CPU-side GeoMesh which may or may not be in memory still.
        /// </summary>
        GeoMesh const& getGeoMesh() const noexcept;

    private:

        [[nodiscard]] bool setGpuData(BufferTypeFlag bufferType, std::span<std::byte const> data, size_t elementSize, GpuBufferHandle& handle) noexcept;
        
        /// <summary>
        /// The object pool that owns the mesh.
        /// </summary>
        ObjectPool* m_pObjectPool;

        /// <summary>
        /// The descriptor that created the mesh.
        /// </summary>
        MeshDescriptor m_descriptor{};

        /// <summary>
        /// The underlying vertex buffer.
        /// </summary>
        GpuBufferHandle m_vertexBufferHandle{};

        /// <summary>
        /// The underlying index buffer.
        /// </summary>
        GpuBufferHandle m_indexBufferHandle{};

        /// <summary>
        /// The CPU copy of the mesh data.
        /// Typically this is only held temporarily until it is uploaded to the GPU.
        /// </summary>
        GeoMesh m_mesh;
    };
}

#endif