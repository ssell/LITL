#include "litl-core/assert.hpp"
#include "litl-engine/objects/mesh.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Mesh::create(Authority<ObjectPool> auth, ObjectPool& pool, MeshDescriptor const& descriptor, ErrorCode& error) noexcept
    {
        LITL_ASSERT_MSG(!m_vertexBufferHandle.isValid() && !m_indexBufferHandle.isValid(), "Attempting to recreate Mesh that has already been created.", false);
        
        m_pObjectPool = &pool;
        m_descriptor = descriptor;

        bool result = setVertices(descriptor.vertexInfo.vertexData, descriptor.vertexInfo.vertexByteSize, false, true, error) &&
                      setIndices(descriptor.indexInfo.indexData, descriptor.indexInfo.indexByteSize, false, true, error);

        m_descriptor.vertexInfo.vertexData = {};
        m_descriptor.indexInfo.indexData = {};

        return result;
    }

    bool Mesh::create(Authority<ObjectPool> auth, ObjectPool& pool, ObjectDescriptor const& descriptor, ErrorCode& error) noexcept
    {
        m_pObjectPool = &pool;
        m_descriptor.objectInfo = descriptor;
        return true;
    }

    void Mesh::destroy(Authority<ObjectPool> auth) noexcept
    {
        m_mesh.clear();
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

    bool Mesh::setVertices(std::span<std::byte const> data, size_t vertexElementSize, bool toCpu, bool toGpu, ErrorCode& error) noexcept
    {
        if (data.empty())
        {
            error = Mesh::ErrorCode::EmptyVertexDataSource;
            return false;
        }

        if (vertexElementSize == 0ull)
        {
            error = Mesh::ErrorCode::InvalidVertexElementSize;
            return false;
        }

        if (toCpu)
        {
            if (vertexElementSize == sizeof(Vertex))
            {
                m_mesh.setVertices(data);
            }
            else
            {
                error = Mesh::ErrorCode::InvalidVertexElementSizeForCpu;
                return false;
            }
        }

        if (toGpu)
        {
            if (setGpuData((BufferTypeFlagBits::VertexBuffer | BufferTypeFlagBits::TransferDest), data, vertexElementSize, m_vertexBufferHandle))
            {
                m_descriptor.vertexInfo.vertexCount = data.size() / vertexElementSize;
                m_descriptor.vertexInfo.vertexByteSize = vertexElementSize;
            }
            else
            {
                error = Mesh::ErrorCode::VertexBufferCreationFailed;
                return false;
            }
        }

        return true;
    }

    bool Mesh::setIndices(std::span<std::byte const> data, size_t indexElementSize, bool toCpu, bool toGpu, ErrorCode& error) noexcept
    {
        if (data.empty())
        {
            error = Mesh::ErrorCode::EmptyIndexDataSource;
            return false;
        }

        if (indexElementSize == 0ull)
        {
            error = Mesh::ErrorCode::InvalidIndexElementSize;
            return false;
        }

        if (toCpu)
        {
            if (indexElementSize == sizeof(uint32_t))
            {
                m_mesh.setIndices(data);
            }
            else
            {
                error = Mesh::ErrorCode::InvalidIndexElementSizeForCpu;
                return false;
            }
        }

        if (toGpu)
        {
            if (setGpuData((BufferTypeFlagBits::IndexBuffer | BufferTypeFlagBits::TransferDest), data, indexElementSize, m_indexBufferHandle))
            {
                m_descriptor.indexInfo.indexCount = data.size() / indexElementSize;
                m_descriptor.indexInfo.indexByteSize = indexElementSize;
            }
            else
            {
                error = Mesh::ErrorCode::IndexBufferCreationFailed;
                return false;
            }
        }

        return true;
    }

    bool Mesh::setGpuData(BufferTypeFlag bufferType, std::span<std::byte const> data, size_t elementSize, GpuBufferHandle& handle) noexcept
    {
        GpuBuffer* buffer = m_pObjectPool->getGpuBuffer(handle);
        size_t currBufferSizeBytes = 0ull;

        if (buffer != nullptr)
        {
            // Need to make sure the index buffer is still valid for the incoming payload.
            const GpuBufferDescriptor& bufferDescriptor = buffer->getDescriptor();
            currBufferSizeBytes = bufferDescriptor.bytes;

            if ((bufferDescriptor.itemBytes != elementSize) || (bufferDescriptor.bytes < data.size_bytes()))
            {
                // The current buffer is incompatible with the new payload. Need to recreate.
                m_pObjectPool->deferDestroyGpuBuffer(handle);
                buffer = nullptr;
            }
        }

        if (buffer == nullptr)
        {
            const GpuBufferDescriptor bufferDescriptor = GpuBufferDescriptor{
               .type = bufferType,
               .memoryUsage = BufferMemoryUsage::GpuOnly,                           // ... todo ...
               .bufferStrategy = GpuBufferingStrategy::Single,
               .bytes = max(data.size_bytes(), currBufferSizeBytes),                // Size up, not down.
               .itemBytes = static_cast<uint32_t>(elementSize)
            };

            handle = m_pObjectPool->createGpuBuffer(bufferDescriptor);
            buffer = m_pObjectPool->getGpuBuffer(handle);

            if (buffer == nullptr)
            {
                return false;
            }
        }

        buffer->setData(data);

        return true;
    }

    bool Mesh::uploadCpuMeshToGpu(ErrorCode& error) noexcept
    {
        return setVertices<Vertex>(m_mesh.getVertices(), false, true, error) &&     // toCpu = false as it is already on the CPU
               setIndices<uint32_t>(m_mesh.getIndices(), false, true, error);
    }

    GeoMesh& Mesh::getGeoMesh() noexcept
    {
        return m_mesh;
    }

    GeoMesh const& Mesh::getGeoMesh() const noexcept
    {
        return m_mesh;
    }
}