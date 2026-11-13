#include "litl-engine/render/passes/shadowPass.hpp"
#include "litl-engine/render/renderManager.hpp"

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
        constexpr RenderTargetOptions renderTargetOptions{
            .loadOp = LoadOperationType::Clear,
            .colorStoreOp = StoreOperationType::DontCare,
            .depthStoreOp = StoreOperationType::Store,
            .hasColorAttachment = false,
            .hasDepthAttachment = true
        };

        viewPushConstants.perPassDataAddr = renderView.perPassDataAddr;
        viewPushConstants.instanceDataAddr = renderView.opaqueInstanceDataAddr;

        if (barrierAndBeginRender(commandBufferHandle, renderView, renderTargetOptions))
        {
            constexpr SetViewportAndScissorCommand setViewportScissorCmd{
                .setViewport = SetViewportCommand {
                    .region = rect2D {
                        .offset = vec2::zero(),
                        .extents = vec2::one()
                    },
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                },
                .setScissor = std::nullopt
            };

            m_pRenderer->cmdSetViewportAndScissor(commandBufferHandle, setViewportScissorCmd);

            // ... todo ...

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
    }
}