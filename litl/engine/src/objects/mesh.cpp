#include "litl-core/assert.hpp"
#include "litl-engine/objects/mesh.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Mesh::create(Authority<ObjectPool> auth, ObjectPool& pool, MeshDescriptor const& descriptor) noexcept
    {
        LITL_ASSERT_MSG(!m_vertexBufferHandle.isValid() && !m_indexBufferHandle.isValid(), "Attempting to recreate Mesh that has already been created.", false);
        
        m_pObjectPool = &pool;
        m_descriptor = descriptor;

        // ---------------------------------------------------------------------------------
        // --- Create Vertex Buffer

        const auto vertexBytesSize = m_descriptor.vertexInfo.vertexCount * m_descriptor.vertexInfo.vertexByteSize;

        m_vertexBufferHandle = m_pObjectPool->createGpuBuffer(GpuBufferDescriptor{
            .type = BufferTypeFlagBits::VertexBuffer,
            .memoryUsage = BufferMemoryUsage::GpuOnly,
            .bufferStrategy = GpuBufferingStrategy::Single,
            .bytes = vertexBytesSize
        });

        auto* vertexBuffer = m_pObjectPool->getGpuBuffer(m_vertexBufferHandle);

        LITL_ASSERT_MSG((vertexBuffer != nullptr), "Failed to create Vertex Buffer for Mesh", false);

        if (m_descriptor.vertexInfo.vertexData.size() > 0ull)
        {
            LITL_ASSERT_MSG((m_descriptor.vertexInfo.vertexData.size() == vertexBytesSize), "Mismatch between provided initial vertex data size and specified buffer size.", false);
            vertexBuffer->setData(m_descriptor.vertexInfo.vertexData);
            m_descriptor.vertexInfo.vertexData = {};
        }

        // ---------------------------------------------------------------------------------
        // --- Create Index Buffer

        const auto indexBytesSize = m_descriptor.indexInfo.indexCount * m_descriptor.indexInfo.indexByteSize;

        m_indexBufferHandle = m_pObjectPool->createGpuBuffer(GpuBufferDescriptor{
            .type = BufferTypeFlagBits::IndexBuffer,
            .memoryUsage = BufferMemoryUsage::GpuOnly,
            .bufferStrategy = GpuBufferingStrategy::Single,
            .bytes = indexBytesSize
        });

        auto* indexBuffer = m_pObjectPool->getGpuBuffer(m_indexBufferHandle);

        LITL_ASSERT_MSG((indexBuffer != nullptr), "Failed to create Index Buffer for Mesh", false);

        if (m_descriptor.indexInfo.indexData.size() > 0ull)
        {
            LITL_ASSERT_MSG((m_descriptor.indexInfo.indexData.size() == indexBytesSize), "Mismatch between provided initial index data size and specified buffer size.", false);
            indexBuffer->setData(m_descriptor.indexInfo.indexData);
            m_descriptor.indexInfo.indexData = {};
        }

        return true;
    }

    void Mesh::destroy(Authority<ObjectPool> auth) noexcept
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

    bool Mesh::setVertices(std::span<std::byte const> data) noexcept
    {
        LITL_ASSERT_MSG((m_pObjectPool != nullptr), "Mesh::setVertices called with a null object pool.", false);

        GpuBuffer* vertexBuffer = m_pObjectPool->getGpuBuffer(m_vertexBufferHandle);

        LITL_ASSERT_MSG((vertexBuffer != nullptr), "Mesh::setVertices called with a null underlying vertex buffer.", false);

        vertexBuffer->setData(data);

        return true;
    }

    bool Mesh::setIndices(std::span<std::byte const> data) noexcept
    {
        LITL_ASSERT_MSG((m_pObjectPool != nullptr), "Mesh::setIndices called with a null object pool.", false);

        GpuBuffer* indexBuffer = m_pObjectPool->getGpuBuffer(m_indexBufferHandle);

        LITL_ASSERT_MSG((indexBuffer != nullptr), "Mesh::setIndices called with a null underlying index buffer.", false);

        indexBuffer->setData(data);

        return true;
    }
}