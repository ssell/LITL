#ifndef LITL_ENGINE_OBJECTS_MESH_H__
#define LITL_ENGINE_OBJECTS_MESH_H__

#include <memory>
#include <span>

#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/mesh.hpp"
#include "litl-engine/types/vertex.hpp"

namespace litl
{
    class ObjectPool;

    struct MeshDescriptor : ObjectDescriptor
    {
        // ... 
    };

    class Mesh
    {
    public:

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

        friend class ObjectPool;

        void create(ObjectPool& pool, MeshDescriptor const& descriptor, GpuBufferHandle vertexBuffer, GpuBufferHandle indexBuffer) noexcept;
        void destroy() noexcept;
        
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