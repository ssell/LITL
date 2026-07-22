#include <chrono>
#include <span>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/render/renderStructs.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-engine/scene/sceneView.hpp"
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
        struct FrameData
        {
            RenderPerFrameData data{};
            GpuBufferHandle handle{};
        };

        struct PassData
        {
            RenderPerPassData data{};
            GpuBufferHandle handle{};
        };

        struct EntityWorldMatrices
        {
            GpuBufferHandle handle{};
        };

        struct DataMap
        {
            RenderDataMap data{};
            GpuBufferHandle handle{};
        };

        std::chrono::steady_clock::time_point startTime;
        std::shared_ptr<ObjectPool> objectPool{ nullptr };
        std::shared_ptr<SceneView> sceneView{ nullptr };
        std::queue<GpuBufferHandle> dirtyBuffers;

        Renderer* renderer{ nullptr };
        RenderPass renderPass{};
        FrameData frameData{};
        PassData passData{};
        EntityWorldMatrices worldMatrices{};
        DataMap dataMap{};
        RenderPushConstants pushConstants{};

        void setup(ServiceProvider& services) noexcept
        {
            startTime = std::chrono::steady_clock::now();
            objectPool = services.get<ObjectPool>();
            sceneView = services.get<SceneView>();

            auto config = services.get<Configuration>();
            auto window = services.get<Window>();

            LITL_FATAL_ASSERT_MSG((objectPool != nullptr), "Failed to inject ObjectPool into RenderManager.");
            LITL_FATAL_ASSERT_MSG((sceneView != nullptr), "Failed to inject SceneView into RenderManager.");
            LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into RenderManager.");
            LITL_FATAL_ASSERT_MSG((window != nullptr), "Failed to inject Window into RenderManager.");

            createRenderer(window.get(), config.get()->rendererSettings);

            LITL_FATAL_ASSERT_MSG((renderer != nullptr), "Failed to create Renderer.");

            frameData.handle = objectPool->createGpuBuffer(GpuBufferDescriptor{
                .type = BufferTypeFlagBits::BufferDeviceAddress,
                .memoryUsage = BufferMemoryUsage::PersistentMap,
                .bufferStrategy = GpuBufferingStrategy::Frame,
                .bytes = sizeof(RenderPerFrameData),
                .itemBytes = sizeof(RenderPerFrameData)
            });

            passData.handle = objectPool->createGpuBuffer(GpuBufferDescriptor{
                .type = BufferTypeFlagBits::BufferDeviceAddress,
                .memoryUsage = BufferMemoryUsage::PersistentMap,
                .bufferStrategy = GpuBufferingStrategy::Frame,
                .bytes = sizeof(RenderPerPassData),
                .itemBytes = sizeof(RenderPerPassData)
            });

            worldMatrices.handle = objectPool->createGpuBuffer(GpuBufferDescriptor{
                .type = BufferTypeFlagBits::BufferDeviceAddress,
                .memoryUsage = BufferMemoryUsage::PersistentMap,
                .bufferStrategy = GpuBufferingStrategy::Frame,
                .bytes = sizeof(mat4) * 1024u,
                .itemBytes = sizeof(mat4),
                .canResize = true
            });

            dataMap.handle = objectPool->createGpuBuffer(GpuBufferDescriptor{
                .type = BufferTypeFlagBits::BufferDeviceAddress,
                .memoryUsage = BufferMemoryUsage::PersistentMap,
                .bufferStrategy = GpuBufferingStrategy::Frame,
                .bytes = sizeof(RenderDataMap),
                .itemBytes = sizeof(RenderDataMap)
            });

            LITL_FATAL_ASSERT_MSG(frameData.handle.isValid(), "Failed to create Frame Data buffer.");
            LITL_FATAL_ASSERT_MSG(passData.handle.isValid(), "Failed to create Pass Data buffer.");
            LITL_FATAL_ASSERT_MSG(dataMap.handle.isValid(), "Failed to create Data Map buffer.");

            renderPass.setup(*renderer, *objectPool);
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
        void onRender(float dt) noexcept
        {
            processDeferredDataTransfers();     // is this where this should live? ... todo ...

            auto const& cullingBucket = CullingSystem::getCombinedCullingBucket();

            if (!renderer->beginRender(MaxRenderWaitTimeMs))
            {
                logWarning("Failed to secure renderer before timing out after ", MaxRenderWaitTimeMs, " ms");
                return;
            }

            auto frameCommandBuffer = renderer->cmdBeginFrame();

            updatePerFrameData(frameCommandBuffer, dt);

            for (auto& renderCamera : cullingBucket.cameraRenderableEntities)
            {
                if (renderCamera.camera == nullptr)
                {
                    break;
                }

                // Only main camera for now to get things working.
                // todo this implementation of "per pass data" via bda wont really work beyond the main camera.
                // the draws are deferred but the changes to the data map buffer (pointing to the appropriate per pass buffer) are immediate

                if (renderCamera.camera->isMainCamera())
                {
                    updatePerPassData(frameCommandBuffer, *renderCamera.camera);
                    renderPass.render(frameCommandBuffer, pushConstants, *renderCamera.camera, renderCamera.entities);

                    break;
                }

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

        /// <summary>
        /// Updates the FrameData buffer which is supplied by default to all shaders via the pushed data map.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="dt"></param>
        void updatePerFrameData(CommandBufferHandle commandBuffer, float dt) noexcept
        {
            auto data = renderer->getFrameData();

            frameData.data.frame = data.frameCount;
            frameData.data.frameIndex = data.frameInFlightIndex;
            frameData.data.deltaTime = dt;
            frameData.data.elapsedTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - startTime).count();

            auto* frameGpuBuffer = objectPool->getGpuBuffer(frameData.handle);
            LITL_ASSERT_MSG((frameGpuBuffer != nullptr), "Attempting to render without a valid frame data buffer.", );

            frameGpuBuffer->swapBuffers(data.frameInFlightIndex);
            frameGpuBuffer->setDataImmediate(as_byte_span(frameData.data), commandBuffer);
            dataMap.data.perFrameDataAddr = frameGpuBuffer->getBufferDeviceAddress().value();
        }

        /// <summary>
        /// Updates the PassData buffer which is supplied by default to all shaders via the pushed data map.
        /// </summary>
        /// <param name="commandBuffer"></param>
        /// <param name="camera"></param>
        void updatePerPassData(CommandBufferHandle commandBuffer, Camera& camera) noexcept
        {
            passData.data.projMatrix = camera.getProjectionMatrix();
            passData.data.viewMatrix = camera.getViewMatrix();
            passData.data.viewProjMatrix = camera.getViewProjectionMatrix();

            auto* passGpuBuffer = objectPool->getGpuBuffer(passData.handle);
            LITL_ASSERT_MSG((passGpuBuffer != nullptr), "Attempting to render without a valid pass data buffer.", );

            passGpuBuffer->swapBuffers(frameData.data.frameIndex);
            passGpuBuffer->setDataImmediate(generic_as_byte_span(&passData.data, sizeof(RenderPerPassData)), commandBuffer);
            dataMap.data.perPassDataAddr = passGpuBuffer->getBufferDeviceAddress().value();
        }

        /// <summary>
        /// Updates the world matrix SSBO whose source is the active SceneTransforms.
        /// </summary>
        /// <param name="commandBuffer"></param>
        void updateWorldMatrices(CommandBufferHandle commandBuffer) noexcept
        {
            auto currWorldMatrices = sceneView->getWorldMatrices();

            auto* worldMatrixBuffer = objectPool->getGpuBuffer(worldMatrices.handle);
            LITL_ASSERT_MSG((worldMatrixBuffer != nullptr), "Attempting to render without a valid world matrix buffer.", );

            worldMatrixBuffer->swapBuffers(frameData.data.frameIndex);

            if (worldMatrixBuffer->getItemCapacity() < currWorldMatrices.size())
            {
                worldMatrixBuffer->resizeItems(currWorldMatrices.size() * 2u, false, true);
            }

            worldMatrixBuffer->setDataImmediate(generic_as_byte_span(currWorldMatrices.data(), currWorldMatrices.size()), commandBuffer);
            dataMap.data.worldMatricesAddr = worldMatrixBuffer->getBufferDeviceAddress().value();
        }

        /// <summary>
        /// Updates the DataMap buffer which is supplied by default to all shaders via the push constants.
        /// </summary>
        /// <param name="commandBuffer"></param>
        void updateDataMapData(CommandBufferHandle commandBuffer) noexcept
        {
            auto* dataMapBuffer = objectPool->getGpuBuffer(dataMap.handle);
            LITL_ASSERT_MSG((dataMapBuffer != nullptr), "Attempting to render without a valid data map buffer.", );

            dataMapBuffer->swapBuffers(frameData.data.frameIndex);
            dataMapBuffer->setDataImmediate(generic_as_byte_span(&dataMap.data, sizeof(RenderDataMap)), commandBuffer);
            pushConstants.dataMapAddr = dataMapBuffer->getBufferDeviceAddress().value();
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

    void RenderManager::onRender(Authority<EngineCallbacks> authority, float dt) noexcept
    {
        m_pImpl->onRender(dt);
    }
}