#include "litl-engine/objects/mesh/mesh.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/containers/common.hpp"

namespace litl
{
    void Mesh::create(ObjectPool& pool, MeshDescriptor const& descriptor, GpuBufferHandle vertexBuffer, GpuBufferHandle indexBuffer) noexcept
    {
        m_pObjectPool = &pool;
        m_descriptor = descriptor;
        m_vertexBufferHandle = vertexBuffer;
        m_indexBufferHandle = indexBuffer;
    }

    void Mesh::destroy() noexcept
    {
        m_pObjectPool->destroyGpuBuffer(m_vertexBufferHandle);
        m_pObjectPool->destroyGpuBuffer(m_indexBufferHandle);
    }

    MeshDescriptor const& Mesh::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    GpuBufferHandle Mesh::getVertexBuffer() const noexcept
    {
        return m_vertexBufferHandle;
    }

    GpuBufferHandle Mesh::getIndexBuffer() const noexcept
    {
        return m_indexBufferHandle;
    }

    bool Mesh::setVertices(std::span<Vertex const> data) noexcept
    {
        LITL_ASSERT_MSG((m_pObjectPool != nullptr), "Mesh::setVertices called with a null object pool.", false);

        GpuBuffer* vertexBuffer = m_pObjectPool->getGpuBuffer(m_vertexBufferHandle);

        LITL_ASSERT_MSG((vertexBuffer != nullptr), "Mesh::setVertices called with a null underlying vertex buffer.", false);

        vertexBuffer->setData(as_byte_span(data));

        return true;
    }

    bool Mesh::setIndices(std::span<uint32_t const> data) noexcept
    {
        LITL_ASSERT_MSG((m_pObjectPool != nullptr), "Mesh::setIndices called with a null object pool.", false);

        GpuBuffer* indexBuffer = m_pObjectPool->getGpuBuffer(m_indexBufferHandle);

        LITL_ASSERT_MSG((indexBuffer != nullptr), "Mesh::setIndices called with a null underlying index buffer.", false);

        indexBuffer->setData(as_byte_span(data));

        return true;
    }
}