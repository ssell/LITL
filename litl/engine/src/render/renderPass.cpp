#include <chrono>

#include "litl-core/assert.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/objects/camera.hpp"

namespace litl
{
    namespace
    {
        static constexpr uint32_t MaxRenderWaitTimeMs = 1000u;
    }

    void RenderPass::setup(ServiceProvider& services) noexcept
    {
        m_pRenderer = services.get<Renderer>();
        LITL_FATAL_ASSERT_MSG((m_pRenderer != nullptr), "Failed to inject Renderer to RenderPass");
    }

    void RenderPass::onRender() noexcept
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

            if (renderCamera.entities.empty())
            {
                continue;
            }

            render(renderCamera.camera, renderCamera.entities);
        }
    }

    void RenderPass::render(Camera* camera, std::vector<Entity> const& entities) noexcept
    {
        if (!camera->isMainCamera())
        {
            // ... only main camera for now just to get things working ...
            return;
        }

        // ... todo ...
    }
}