#include "litl-core/assert.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"

namespace litl
{
    void RenderPass::setup(ServiceProvider& services) noexcept
    {
        m_pRenderer = services.get<Renderer>();
        LITL_FATAL_ASSERT_MSG((m_pRenderer != nullptr), "Failed to inject Renderer to RenderPass");
    }

    void RenderPass::onRender() noexcept
    {
        auto const& cullingBucket = CullingSystem::getCombinedCullingBucket();

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

    }
}