#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"

#ifdef LITL_RENDERER_VULKAN
#include "litl-renderer-vulkan/integration.hpp"
#endif

#ifdef LITL_RENDERER_D3D12
#include "litl-renderer-d3d12/integration.hpp"
#endif

#ifdef LITL_RENDERER_METAL
#include "litl-renderer-metal/integration.hpp"
#endif

namespace litl
{
    namespace
    {
        static constexpr uint32_t MaxRenderWaitTimeMs = 1000u;
    }

    RenderManager::RenderManager()
    {

    }

    RenderManager::~RenderManager()
    {
        // ... Required since the ServiceProvider stores this in a std::shared_ptr ...
    }

    void RenderManager::setup(Authority<Engine> authority, ServiceProvider& services) noexcept
    {
        m_pObjectPool = services.get<ObjectPool>();
        auto config = services.get<Configuration>();
        auto window = services.get<Window>();

        LITL_FATAL_ASSERT_MSG((m_pObjectPool != nullptr), "Failed to inject ObjectPool into RenderManager");
        LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into RenderManager");
        LITL_FATAL_ASSERT_MSG((window != nullptr), "Failed to inject Window into RenderManager");

        createRenderer(window.get(), config.get()->rendererSettings);
    }

    void RenderManager::createRenderer(Window* window, RendererConfiguration const& rendererDescriptor) noexcept
    {
        switch (rendererDescriptor.rendererType)
        {
#ifdef LITL_RENDERER_VULKAN
        case RendererBackendType::Vulkan:
            m_pRenderer = createVulkanRenderer(window, rendererDescriptor);
            break;
#endif

#ifdef LITL_RENDERER_D3D12
        case RendererBackendType::D3D12:
            m_pRenderer = createD3D12Renderer(window, rendererDescriptor);
            break;
#endif

#ifdef LITL_RENDERER_METAL
        case RendererBackendType::Metal:
            m_pRenderer = createMetalRenderer(window, rendererDescriptor);
            break;
#endif

        default:
            break;
        }

        LITL_FATAL_ASSERT_MSG((m_pRenderer != nullptr), "Failed to create Renderer in RenderManager");

        m_pRenderer->build();
    }

    Renderer const* RenderManager::getRenderer() const noexcept
    {
        return m_pRenderer;
    }

    void RenderManager::trackDirtyBuffer(Authority<GpuBuffer> auth, GpuBufferHandle handle) noexcept
    {
        m_dirtyBuffers.push(handle);
    }

    void RenderManager::processDeferredDataTransfers() noexcept
    {
        if (m_dirtyBuffers.empty())
        {
            return;
        }

        // todo: update this to use jobs in the future. doing single-threaded for the moment just to get it working.
        logTrace("Processing ", m_dirtyBuffers.size(), " deferred data transfers ...");

        {
            auto scopedCommandBuffer = m_pRenderer->createScopedCommandBuffer();

            while (!m_dirtyBuffers.empty())
            {
                auto gpuBufferHandle = m_dirtyBuffers.front(); m_dirtyBuffers.pop();
                auto* gpuBuffer = m_pObjectPool->getGpuBuffer(gpuBufferHandle);

                if (gpuBuffer != nullptr)
                {
                    gpuBuffer->flushData({}, scopedCommandBuffer.get());
                }
            }
        }
    }

    void RenderManager::sortVisibleEntities(CullingBucket& cullingBucket) noexcept
    {
        // Do a single-thread sequential sort of all camera entities for now.
        // Profile in the future under stress tests and determine if we need to split work into jobs.

        for (auto& renderCamera : cullingBucket.cameraRenderableEntities)
        {
            if (renderCamera.camera == nullptr)
            {
                break;
            }

            std::sort(renderCamera.entities.begin(), renderCamera.entities.end(), [](RenderableEntity const& a, RenderableEntity const& b) -> bool {
                // Simple sort by material and then mesh for now. In the future will need to check if the material is transparent so we can also sort from far to near.
                // This sort will group all entities of the same material and mesh together. Example result:
                //     [(mat0, mesh0), (mat0, mesh0), (mat0, mesh3), (mat1, mesh2), (mat1, mesh2), (mat1, mesh4), (mat2, mesh5)]

                if (a.material.handle.index < b.material.handle.index)
                {
                    return true;
                }
                else
                {
                    return (a.mesh.handle.index < b.material.handle.index);
                }
            });
        }
    }

    void RenderManager::onRender(Authority<EngineCallbacks> authority) noexcept
    {
        processDeferredDataTransfers();     // is this where this should live? ... todo ...

        auto const& cullingBucket = CullingSystem::getCombinedCullingBucket();

        if (!m_pRenderer->beginRender(MaxRenderWaitTimeMs))
        {
            logWarning("Failed to secure renderer before timing out after ", MaxRenderWaitTimeMs, " ms");
            return;
        }

        auto frameCommandBuffer = m_pRenderer->cmdBeginFrame();

        for (auto& renderCamera : cullingBucket.cameraRenderableEntities)
        {
            if (renderCamera.camera == nullptr)
            {
                break;
            }

            m_renderPass.render(*m_pRenderer, frameCommandBuffer, *m_pObjectPool, renderCamera.camera, renderCamera.entities);
        }

        m_pRenderer->endRender();
    }
}