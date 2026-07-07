#include <memory>

#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    struct ObjectPool::Impl
    {
        std::shared_ptr<Renderer> renderer;

        HandlePool<Camera, CameraHandleTag> cameraPool;
        HandlePool<GpuBuffer, GpuBufferHandleTag> gpuBufferPool;
        HandlePool<Mesh, MeshHandleTag> meshPool;
    };

    ObjectPool::ObjectPool()
    {

    }

    ObjectPool::~ObjectPool()
    {
        // ... Required since the ServiceProvider stores this in a std::shared_ptr ...
    }

    void ObjectPool::setup(ServiceProvider& services) noexcept
    {
        m_impl->renderer = services.get<Renderer>();
    }

    void ObjectPool::destroy() noexcept
    {
        logInfo("Destroying ObjectPool ...");

        // ---- Cameras

        std::vector<CameraHandle> cameraHandles;
        m_impl->cameraPool.getAllHandles(cameraHandles);

        logTrace("... destroying ", cameraHandles.size(), " Camera handles.");

        for (auto cameraHandle : cameraHandles)
        {
            destroyCamera(cameraHandle);
        }

        // ---- Meshes

        std::vector<MeshHandle> meshHandles;
        m_impl->meshPool.getAllHandles(meshHandles);

        logTrace("... destroying ", meshHandles.size(), " Mesh handles.");

        for (auto meshHandle : meshHandles)
        {
            destroyMesh(meshHandle);
        }

        // ---- GPU Buffers

        std::vector<GpuBufferHandle> gpuBufferHandles;
        m_impl->gpuBufferPool.getAllHandles(gpuBufferHandles);

        logTrace("... destroying ", gpuBufferHandles.size(), " GPU Buffer handles.");

        for (auto gpuBufferHandle : gpuBufferHandles)
        {
            destroyGpuBuffer(gpuBufferHandle);
        }
    }

    //--------------------------------------------------------------------------------------
    // Camera
    //--------------------------------------------------------------------------------------

    CameraHandle ObjectPool::createCamera(CameraDescriptor const& descriptor) noexcept
    {
        Camera camera{};
        camera.create(descriptor);
        return m_impl->cameraPool.create(camera);
    }

    Camera* ObjectPool::getCamera(CameraHandle handle) noexcept
    {
        return m_impl->cameraPool.get(handle);
    }

    void ObjectPool::destroyCamera(CameraHandle handle) noexcept
    {
        Camera* camera = getCamera(handle);

        if (camera != nullptr)
        {
            camera->destroy();
            m_impl->cameraPool.destroy(handle);
        }
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
            buffer->destroy();
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

        mesh.create(*this, descriptor, vertexBuffer, indexBuffer);

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
            mesh->destroy();
            m_impl->meshPool.destroy(handle);
        }
    }
}