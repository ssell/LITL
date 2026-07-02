#include <memory>

#include "litl-engine/objects/objectPool.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    struct ObjectPool::Impl
    {
        std::shared_ptr<Renderer> renderer;
        HandlePool<GpuBuffer, GpuBufferHandleTag> gpuBufferPool;
        HandlePool<Mesh, MeshHandleTag> meshPool;
    };

    ObjectPool::ObjectPool()
    {

    }

    ObjectPool::~ObjectPool()
    {

    }

    void ObjectPool::setup(ServiceProvider& services) noexcept
    {
        m_impl->renderer = services.get<Renderer>();
    }

    //--------------------------------------------------------------------------------------
    // GpuBuffer
    //--------------------------------------------------------------------------------------

    GpuBufferHandle ObjectPool::createGpuBuffer(GpuBufferDescriptor const& descriptor) noexcept
    {
        GpuBuffer buffer{};
        buffer.create(descriptor);
        return m_impl->gpuBufferPool.create(buffer);
    }

    GpuBuffer* ObjectPool::getGpuBuffer(GpuBufferHandle handle) noexcept
    {
        return m_impl->gpuBufferPool.get(handle);
    }

    void ObjectPool::destroyGpuBuffer(GpuBufferHandle handle) noexcept
    {
        GpuBuffer* buffer = getGpuBuffer(handle);

        if (buffer != nullptr)
        {
            // ... todo ...

            m_impl->gpuBufferPool.destroy(handle);
        }
    }

    //--------------------------------------------------------------------------------------
    // Mesh
    //--------------------------------------------------------------------------------------

    MeshHandle ObjectPool::createMesh(MeshDescriptor const& descriptor) noexcept
    {
        Mesh mesh{};

        const GpuBufferDescriptor vertexBufferDescriptor{
            // ... todo ...
        };

        const GpuBufferDescriptor indexBufferDescriptor{
            // ... todo ...
        };

        const GpuBufferHandle vertexBuffer = createGpuBuffer(vertexBufferDescriptor);
        const GpuBufferHandle indexBuffer = createGpuBuffer(indexBufferDescriptor);

        LITL_ASSERT_MSG((vertexBuffer.isValid() && indexBuffer.isValid()), "Failed to create vertex and/or index buffer for mesh.", {});

        mesh.create(this, descriptor, vertexBuffer, indexBuffer);

        return m_impl->meshPool.create(mesh);
    }

    Mesh* ObjectPool::getMesh(MeshHandle handle) noexcept
    {
        return m_impl->meshPool.get(handle);
    }

    void ObjectPool::destroyMesh(MeshHandle handle) noexcept
    {
        Mesh* mesh = getMesh(handle);

        if (mesh != nullptr)
        {
            // ... todo ...

            m_impl->meshPool.destroy(handle);
        }
    }
}