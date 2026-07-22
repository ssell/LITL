#include <memory>

#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/scene/sceneView.hpp"

namespace litl
{
    struct ObjectPool::Impl
    {
        std::shared_ptr<RenderManager> renderManager;
        std::shared_ptr<World> world;
        std::shared_ptr<SceneView> sceneView;

        HandlePool<Camera, CameraHandleTag> cameraPool;
        HandlePool<GpuBuffer, GpuBufferHandleTag> gpuBufferPool;
        HandlePool<Material, MaterialHandleTag> materialPool;
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
        m_impl->sceneView = services.get<SceneView>();

        LITL_FATAL_ASSERT_MSG((m_impl->renderManager != nullptr), "Failed to inject RenderManager to ObjectPool");
        LITL_FATAL_ASSERT_MSG((m_impl->world != nullptr), "Failed to inject World to ObjectPool");
        LITL_FATAL_ASSERT_MSG((m_impl->sceneView != nullptr), "Failed to inject SceneView to ObjectPool");
    }

    void ObjectPool::destroy() noexcept
    {
        logInfo("Destroying ObjectPool ...");

        // ---- Cameras

        std::vector<CameraHandle> cameraHandles;
        getAllCameraHandles(cameraHandles);

        logTrace("... destroying ", cameraHandles.size(), " Camera handles.");

        for (auto cameraHandle : cameraHandles)
        {
            destroyCamera(cameraHandle);
        }

        // ---- Materials

        std::vector<MaterialHandle> materialHandles;
        getAllMaterialHandles(materialHandles);

        logTrace("... destroying ", materialHandles.size(), " Material handles.");

        for (auto materialHandle : materialHandles)
        {
            destroyMaterial(materialHandle);
        }

        // ---- Meshes

        std::vector<MeshHandle> meshHandles;
        getAllMeshHandles(meshHandles);

        logTrace("... destroying ", meshHandles.size(), " Mesh handles.");

        for (auto meshHandle : meshHandles)
        {
            destroyMesh(meshHandle);
        }

        // ---- GPU Buffers

        std::vector<GpuBufferHandle> gpuBufferHandles;
        getAllGpuBufferHandles(gpuBufferHandles);

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

        cameraRef->create({}, descriptor, *m_impl->world, *m_impl->sceneView, handle);

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
        
        if (!buffer.create({}, descriptor, *m_impl->renderManager.get()))
        {
            logWarning("Failed to create GPU Buffer '", descriptor.objectInfo.name, "'");
            buffer.destroy({});     // make sure there are no lingering resources depending on when in the creation process the error occurred.
            return {};
        }

        auto handle = m_impl->gpuBufferPool.create(buffer);
        auto* bufferPtr = m_impl->gpuBufferPool.get(handle);

        if (bufferPtr != nullptr)
        {
            bufferPtr->setBufferHandle({}, handle);
        }

        return handle;
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
    // Material
    //--------------------------------------------------------------------------------------

    MaterialHandle ObjectPool::createMaterial(MaterialDescriptor const& descriptor) noexcept
    {
        Material material{};

        if (!material.create({}, descriptor, *(m_impl->renderManager->getRenderer())))
        {
            logWarning("Failed to create Material '", descriptor.objectInfo.name, "'");
            material.destroy({});
            return {};
        }

        return m_impl->materialPool.create(material);
    }

    Material* ObjectPool::getMaterial(MaterialHandle handle) noexcept
    {
        return m_impl->materialPool.get(handle);
    }

    void ObjectPool::getAllMaterialHandles(std::vector<MaterialHandle>& handles) const noexcept
    {
        m_impl->materialPool.getAllHandles(handles);
    }

    void ObjectPool::destroyMaterial(MaterialHandle handle) noexcept
    {
        Material* material = getMaterial(handle);

        if (material != nullptr)
        {
            material->destroy({});
            m_impl->materialPool.destroy(handle);
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
            logWarning("Failed to create Mesh '", descriptor.objectInfo.name, "'");
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