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
        const RenderTargetOptions renderTargetOptions{
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
            m_pRenderer->cmdSetViewportAndScissor(commandBufferHandle, createSetViewportAndScissorCommand(renderView.camera, false));

            if (compileRenderList(renderView.sortedOpaqueEntities))
            {
                drawRenderList(commandBufferHandle, viewPushConstants, DrawListOptions{ .requiresMaterialProperties = false });
            }

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
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