#include <memory>

#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderManager.hpp"

namespace litl
{
    struct ObjectPool::Impl
    {
        std::shared_ptr<RenderManager> renderManager;
        std::shared_ptr<World> world;

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
        m_impl->renderManager = services.get<RenderManager>();
        m_impl->world = services.get<World>();

        LITL_FATAL_ASSERT_MSG((m_impl->renderManager != nullptr), "Failed to inject RenderManager to ObjectPool");
        LITL_FATAL_ASSERT_MSG((m_impl->world != nullptr), "Failed to inject World to ObjectPool");
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
        auto handle = m_impl->cameraPool.create({});
        auto* cameraRef = m_impl->cameraPool.get(handle);

        cameraRef->create({}, descriptor, *(m_impl->world.get()), handle);

        return handle;
    }

    Camera* ObjectPool::getCamera(CameraHandle handle) noexcept
    {
        return m_impl->cameraPool.get(handle);
    }

    void ObjectPool::getAllCameraHandles(std::vector<CameraHandle>& handles) const noexcept
    {
        m_impl->cameraPool.getAllHandles(handles);
    }

    void ObjectPool::destroyCamera(CameraHandle handle) noexcept
    {
        Camera* camera = getCamera(handle);

        if (camera != nullptr)
        {
            camera->destroy({});
            m_impl->cameraPool.destroy(handle);
        }
    }

    //--------------------------------------------------------------------------------------
    // GpuBuffer
    //--------------------------------------------------------------------------------------

    GpuBufferHandle ObjectPool::createGpuBuffer(GpuBufferDescriptor const& descriptor) noexcept
    {
        GpuBuffer buffer{};
        
        if (!buffer.create({}, descriptor, m_impl->renderManager->getRenderer()))
        {
            buffer.destroy({});     // make sure there are no lingering resources depending on when in the creation process the error occurred.
            return {};
        }

        return m_impl->gpuBufferPool.create(buffer);
    }

    GpuBuffer* ObjectPool::getGpuBuffer(GpuBufferHandle handle) noexcept
    {
        return m_impl->gpuBufferPool.get(handle);
    }

    void ObjectPool::getAllGpuBufferHandles(std::vector<GpuBufferHandle>& handles) const noexcept
    {
        m_impl->gpuBufferPool.getAllHandles(handles);
    }

    void ObjectPool::destroyGpuBuffer(GpuBufferHandle handle) noexcept
    {
        GpuBuffer* buffer = getGpuBuffer(handle);

        if (buffer != nullptr)
        {
            buffer->destroy({});
            m_impl->gpuBufferPool.destroy(handle);
        }
    }

    //--------------------------------------------------------------------------------------
    // Mesh
    //--------------------------------------------------------------------------------------

    MeshHandle ObjectPool::createMesh(MeshDescriptor const& descriptor) noexcept
    {
        Mesh mesh{};
        
        if (!mesh.create({}, *this, descriptor))
        {
            mesh.destroy({});       // make sure there are no lingering resources depending on when in the creation process the error occurred.
            return {};
        }

        return m_impl->meshPool.create(mesh);
    }

    Mesh* ObjectPool::getMesh(MeshHandle handle) noexcept
    {
        return m_impl->meshPool.get(handle);
    }

    void ObjectPool::getAllMeshHandles(std::vector<MeshHandle>& handles) const noexcept
    {
        m_impl->meshPool.getAllHandles(handles);
    }

    void ObjectPool::destroyMesh(MeshHandle handle) noexcept
    {
        Mesh* mesh = getMesh(handle);

        if (mesh != nullptr)
        {
            mesh->destroy({});
            m_impl->meshPool.destroy(handle);
        }
    }
}