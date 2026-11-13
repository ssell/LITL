#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderPassContext.hpp"
#include "litl-engine/render/passes/forwardTransparentPass.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    void ForwardTransparentPass::render(RenderPassContext const& context) noexcept
    {
        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.type == RenderViewType::Camera)
            {
                render(context.commandBufferHandle, renderView, context.pushConstants);
            }
        }
    }

    void ForwardTransparentPass::render(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept
    {
        constexpr RenderTargetOptions renderTargetOptions{
            .loadOp = LoadOperationType::Load,
            .colorStoreOp = StoreOperationType::Store,
            .depthStoreOp = StoreOperationType::DontCare,
            .hasColorAttachment = true,
            .hasDepthAttachment = true
        };

        viewPushConstants.perPassDataAddr = renderView.perPassDataAddr;
        viewPushConstants.instanceDataAddr = renderView.transparentInstanceDataAddr;

        if (barrierAndBeginRender(commandBufferHandle, renderView, renderTargetOptions))
        {
            m_pRenderer->cmdSetViewportAndScissor(commandBufferHandle, createSetViewportAndScissorCommand(renderView, false));

            if (compileRenderList(renderView.sortedTransparentEntities))
            {
                drawRenderList(commandBufferHandle, viewPushConstants, DrawListOptions{ .requiresMaterialProperties = true });
            }

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
    }
}