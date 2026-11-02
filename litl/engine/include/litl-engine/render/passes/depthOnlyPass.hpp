#ifndef LITL_ENGINE_RENDER_DEPTH_ONLY_PASS_H__
#define LITL_ENGINE_RENDER_DEPTH_ONLY_PASS_H__

#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    class DepthOnlyPass : public RenderPass
    {
    public:

        void render(RenderPassContext const& context) noexcept override;

    protected:

        void render(CommandBufferHandle commandBufgferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept;
        [[nodiscard]] bool barrierAndBeginRender(CommandBufferHandle commandBufferHandle, RenderView const& renderView) noexcept;
        [[nodiscard]] GraphicsPipelineHandle resolveGraphicsPipelineHandle(Material* material) const noexcept override;
    };
}

#endif