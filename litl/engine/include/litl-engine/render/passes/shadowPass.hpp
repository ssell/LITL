#ifndef LITL_ENGINE_RENDER_SHADOW_PASS_H__
#define LITL_ENGINE_RENDER_SHADOW_PASS_H__

#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    class ShadowPass : public RenderPass
    {
    public:

        void render(RenderPassContext const& context) noexcept override;

    protected:

        [[nodiscard]] GraphicsPipelineHandle resolveGraphicsPipelineHandle(Material* material) const noexcept override;
        [[nodiscard]] bool isValidRenderPassMaterial(Material const& material) const noexcept override;
    private:

        void render(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept;
    };
}

#endif