#include "litl-engine/render/passes/depthOnlyPass.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/objects/material/material.hpp"

namespace litl
{
    void DepthOnlyPass::render(RenderPassContext const& context) noexcept
    {
        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.type == RenderViewType::DepthOnly)
            {
                render(context.commandBufferHandle, renderView, context.pushConstants);
            }
        }
    }

    void DepthOnlyPass::render(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept
    {
        viewPushConstants.perPassDataAddr = renderView.perPassDataAddr;
        viewPushConstants.instanceDataAddr = renderView.instanceDataAddr;

        if (barrierAndBeginRender(commandBufferHandle, renderView))
        {
            m_pRenderer->cmdSetViewportAndScissor(commandBufferHandle, createSetViewportAndScissorCommand(renderView.camera, false));

            if (compileRenderList(renderView.entities))
            {
                drawRenderList(commandBufferHandle, viewPushConstants, DrawListOptions{ .requiresMaterialProperties = false });
            }

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
    }

    bool DepthOnlyPass::barrierAndBeginRender(CommandBufferHandle commandBufferHandle, RenderView const& renderView) noexcept
    {
        const RenderTargetOptions renderTargetOptions{
            .loadOp = LoadOperationType::Clear,
            .colorStoreOp = StoreOperationType::DontCare,
            .depthStoreOp = StoreOperationType::Store,
            .hasColorAttachment = false,
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

    GraphicsPipelineHandle DepthOnlyPass::resolveGraphicsPipelineHandle(Material* material) const noexcept
    {
        constexpr RenderAttachmentFormats formats{
            .color = DataFormat::Undefined,
            .depth = DataFormat::D32_SFloat
        };

        return material->getGraphicsPipelineHandle(MaterialVariantType::DepthOnly, formats, true);
    }
}