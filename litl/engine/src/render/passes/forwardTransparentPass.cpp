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
        viewPushConstants.perPassDataAddr = renderView.perPassDataAddr;
        viewPushConstants.instanceDataAddr = renderView.transparentInstanceDataAddr;

        if (barrierAndBeginRender(commandBufferHandle, renderView))
        {
            m_pRenderer->cmdSetViewportAndScissor(commandBufferHandle, createSetViewportAndScissorCommand(renderView.camera, false));

            if (compileRenderList(renderView.sortedTransparentEntities))
            {
                drawRenderList(commandBufferHandle, viewPushConstants, DrawListOptions{ .requiresMaterialProperties = true });
            }

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
    }

    bool ForwardTransparentPass::barrierAndBeginRender(CommandBufferHandle commandBufferHandle, RenderView const& renderView) noexcept
    {
        const RenderTargetOptions renderTargetOptions{
            .loadOp = LoadOperationType::Load,
            .colorStoreOp = StoreOperationType::Store,
            .depthStoreOp = StoreOperationType::DontCare,
            .hasColorAttachment = true,
            .hasDepthAttachment = true
        };

        const auto renderTarget = resolveRenderTarget(renderView.camera, renderView.renderTexture, renderTargetOptions);

        if (!renderTarget.valid)
        {
            return false;
        }

        for (const auto& pipelineBarrierCmd : renderTarget.getPipelineBarrierCmds())
        {
            m_pRenderer->cmdPipelineBarrier(commandBufferHandle, pipelineBarrierCmd);
        }

        return m_pRenderer->cmdBeginRender(commandBufferHandle, renderTarget.beginRenderCmd);
    }
}