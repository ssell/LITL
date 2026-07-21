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

    struct RenderManager::Impl
    {
        std::shared_ptr<ObjectPool> objectPool{ nullptr };
        std::queue<GpuBufferHandle> dirtyBuffers;
        Renderer* renderer{ nullptr };
        RenderPass renderPass{};

        void setup(ServiceProvider& services) noexcept
        {
            objectPool = services.get<ObjectPool>();
            auto config = services.get<Configuration>();
            auto window = services.get<Window>();

            LITL_FATAL_ASSERT_MSG((objectPool != nullptr), "Failed to inject ObjectPool into RenderManager");
            LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into RenderManager");
            LITL_FATAL_ASSERT_MSG((window != nullptr), "Failed to inject Window into RenderManager");

            createRenderer(window.get(), config.get()->rendererSettings);
        }

        void createRenderer(Window* window, RendererConfiguration const& rendererDescriptor) noexcept
        {
            switch (rendererDescriptor.rendererType)
            {
#ifdef LITL_RENDERER_VULKAN
            case RendererBackendType::Vulkan:
                renderer = createVulkanRenderer(window, rendererDescriptor);
                break;
#endif

#ifdef LITL_RENDERER_D3D12
            case RendererBackendType::D3D12:
                renderer = createD3D12Renderer(window, rendererDescriptor);
                break;
#endif

#ifdef LITL_RENDERER_METAL
            case RendererBackendType::Metal:
                renderer = createMetalRenderer(window, rendererDescriptor);
                break;
#endif

            default:
                break;
            }

            LITL_FATAL_ASSERT_MSG((renderer != nullptr), "Failed to create Renderer in RenderManager");

            renderer->build();
        }

        void trackDirtyBuffer(GpuBufferHandle handle) noexcept
        {
            dirtyBuffers.push(handle);
        }

        /// <summary>
        /// Invoked once per frame to render the scene.
        /// </summary>
        void onRender() noexcept
        {
            processDeferredDataTransfers();     // is this where this should live? ... todo ...

            auto const& cullingBucket = CullingSystem::getCombinedCullingBucket();

            if (!renderer->beginRender(MaxRenderWaitTimeMs))
            {
                logWarning("Failed to secure renderer before timing out after ", MaxRenderWaitTimeMs, " ms");
                return;
            }

            auto frameCommandBuffer = renderer->cmdBeginFrame();

            for (auto& renderCamera : cullingBucket.cameraRenderableEntities)
            {
                if (renderCamera.camera == nullptr)
                {
                    break;
                }

                renderPass.render(*renderer, frameCommandBuffer, *objectPool, renderCamera.camera, renderCamera.entities);
            }

            renderer->endRender();
        }

        /// <summary>
        /// Performs all deferred buffer data transfers from CPU to GPU.
        /// </summary>
        void processDeferredDataTransfers() noexcept
        {
            if (dirtyBuffers.empty())
            {
                return;
            }

            // todo: update this to use jobs in the future. doing single-threaded for the moment just to get it working.
            logTrace("Processing ", dirtyBuffers.size(), " deferred data transfers ...");

            {
                auto scopedCommandBuffer = renderer->createScopedCommandBuffer();

                while (!dirtyBuffers.empty())
                {
                    auto gpuBufferHandle = dirtyBuffers.front(); dirtyBuffers.pop();
                    auto* gpuBuffer = objectPool->getGpuBuffer(gpuBufferHandle);

                    if (gpuBuffer != nullptr)
                    {
                        gpuBuffer->flushData({}, scopedCommandBuffer.get());
                    }
                }
            }
        }

        /// <summary>
        /// Sorts entities by material, mesh, etc. to build up their render order.
        /// </summary>
        /// <param name="cullingBucket"></param>
        void sortVisibleEntities(CullingBucket& cullingBucket) noexcept
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
    };

    RenderManager::RenderManager()
        : m_pImpl(std::make_unique<RenderManager::Impl>())
    {

    }

    RenderManager::~RenderManager()
    {
        // ... Required since the ServiceProvider stores this in a std::shared_ptr ...
    }

    void RenderManager::setup(Authority<Engine> authority, ServiceProvider& services) noexcept
    {
        m_pImpl->setup(services);
    }

    Renderer const* RenderManager::getRenderer() const noexcept
    {
        return m_pImpl->renderer;
    }

    void RenderManager::trackDirtyBuffer(Authority<GpuBuffer> auth, GpuBufferHandle handle) noexcept
    {
        m_pImpl->trackDirtyBuffer(handle);
    }

    void RenderManager::onRender(Authority<EngineCallbacks> authority) noexcept
    {
        m_pImpl->onRender();
    }
}