#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderPassContext.hpp"
#include "litl-engine/render/passes/forwardOpaquePass.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    void ForwardOpaquePass::render(RenderPassContext const& context) noexcept
    {
        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.type == RenderViewType::Camera)
            {
                render(context.commandBufferHandle, renderView, context.pushConstants);
            }
        }
    }

    void ForwardOpaquePass::render(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept
    {
        viewPushConstants.perPassDataAddr = renderView.perPassDataAddr;
        viewPushConstants.instanceDataAddr = renderView.instanceDataAddr;

        if (barrierAndBeginRender(commandBufferHandle, renderView))
        {
            m_pRenderer->cmdSetViewportAndScissor(commandBufferHandle, createSetViewportAndScissorCommand(renderView.camera, false));

            if (compileRenderList(renderView.entities))
            {
                drawRenderList(commandBufferHandle, viewPushConstants, DrawListOptions{ .requiresMaterialProperties = true });
            }

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
    }

    bool ForwardOpaquePass::barrierAndBeginRender(CommandBufferHandle commandBufferHandle, RenderView const& renderView) noexcept
    {
        const RenderTargetOptions renderTargetOptions{
            .loadOp = LoadOperationType::Clear,                     // Will need to change if/when ForwardOpaquePass is not the first pass to write to the camera render textures
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

        for (auto const& pipelineBarrierCmd : renderTarget.getPipelineBarrierCmds())
        {
            m_pRenderer->cmdPipelineBarrier(commandBufferHandle, pipelineBarrierCmd);
        }

        return m_pRenderer->cmdBeginRender(commandBufferHandle, renderTarget.beginRenderCmd);
    }
}