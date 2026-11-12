#include "litl-engine/render/passes/shadowPass.hpp"

namespace litl
{
    void ShadowPass::render(RenderPassContext const& context) noexcept
    {
        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.type == RenderViewType::Shadow)
            {
                render(context.commandBufferHandle, renderView, context.pushConstants);
            }
        }
    }

    void ShadowPass::render(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept
    {

    }
}