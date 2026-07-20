#ifndef LITL_ENGINE_OBJECTS_MESH_H__
#define LITL_ENGINE_OBJECTS_MESH_H__

#include <memory>
#include <span>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/mesh.hpp"
#include "litl-engine/types/vertex.hpp"
#include "litl-core/containers/common.hpp"      // not needed directly, but contains as_byte_span which is typically used to provide vertex/index byte data

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

    struct MeshDescriptor : ObjectDescriptor
    {
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

        /// <summary>
        /// 
        /// </summary>
        /// <param name="auth"></param>
        /// <param name="pool"></param>
        /// <param name="descriptor"></param>
        /// <param name="vertexBuffer"></param>
        /// <param name="indexBuffer"></param>
        void create(Authority<ObjectPool> auth, ObjectPool& pool, MeshDescriptor const& descriptor, GpuBufferHandle vertexBuffer, GpuBufferHandle indexBuffer) noexcept;

        /// <summary>
        /// 
        /// </summary>
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// Retrieves the descriptor that was used to create this mesh.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] MeshDescriptor const& getDescriptor() const noexcept;

        /// <summary>
        /// Retrieves the handle of the underlying vertex buffer.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] GpuBufferHandle getVertexBuffer() const noexcept;

        /// <summary>
        /// Retrieves the handle of the underlying index buffer.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] GpuBufferHandle getIndexBuffer() const noexcept;

        /// <summary>
        /// Sets the vertices for the mesh.
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        bool setVertices(std::span<Vertex const> data) noexcept;

        /// <summary>
        /// Sets the indices for the mesh.
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        bool setIndices(std::span<uint32_t const> data) noexcept;

    private:
        
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
    };
}

#endif