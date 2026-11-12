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
        const RenderTargetOptions renderTargetOptions{
            .loadOp = LoadOperationType::Clear,                     // Will need to change if/when ForwardOpaquePass is not the first pass to write to the camera render textures
            .colorStoreOp = StoreOperationType::Store,
            .depthStoreOp = StoreOperationType::DontCare,
            .hasColorAttachment = true,
            .hasDepthAttachment = true
        };

        viewPushConstants.perPassDataAddr = renderView.perPassDataAddr;
        viewPushConstants.instanceDataAddr = renderView.opaqueInstanceDataAddr;

        if (barrierAndBeginRender(commandBufferHandle, renderView, renderTargetOptions))
        {
            m_pRenderer->cmdSetViewportAndScissor(commandBufferHandle, createSetViewportAndScissorCommand(renderView.camera, false));

            if (compileRenderList(renderView.sortedOpaqueEntities))
            {
                drawRenderList(commandBufferHandle, viewPushConstants, DrawListOptions{ .requiresMaterialProperties = true });
            }

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
    }
}