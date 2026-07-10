#include "litl-core/assert.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/engineCallbacks.hpp"
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
        auto config = services.get<Configuration>();
        auto window = services.get<Window>();

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

            // Simple sort by material and then mesh for now.
            // In the future will need to check if the material is transparent so we can also sort from far to near.

        }
    }

    void RenderManager::onRender(Authority<EngineCallbacks> authority) noexcept
    {
        auto const& cullingBucket = CullingSystem::getCombinedCullingBucket();

        if (!m_pRenderer->beginRender(MaxRenderWaitTimeMs))
        {
            logWarning("Failed to secure renderer before timing out after ", MaxRenderWaitTimeMs, " ms");
            return;
        }

        for (auto& renderCamera : cullingBucket.cameraRenderableEntities)
        {
            if (renderCamera.camera == nullptr)
            {
                break;
            }

            m_renderPass.render(m_pRenderer, renderCamera.camera, renderCamera.entities);
        }

        m_pRenderer->endRender();
    }
}