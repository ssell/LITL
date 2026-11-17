#include "litl-engine/render/passes/shadowPass.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    void ShadowPass::render(RenderPassContext const& context) noexcept
    {
        // Render into the shadow maps
        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.type == RenderViewType::Shadow)
            {
                render(context.commandBufferHandle, renderView, context.pushConstants);
            }
        }

        // Barrier the shadow maps
        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.type != RenderViewType::Shadow)
            {
                continue;
            }

            const Texture* shadowPoolTexture = m_pObjectPool->getTexture(renderView.depthTargetOverrideHandle);

            if (shadowPoolTexture == nullptr)
            {
                continue;
            }

            m_pRenderer->cmdPipelineBarrier(context.commandBufferHandle, PipelineBarrierCommand::copyWithTexture(
                PipelineBarrierDepthToShaderRead,
                shadowPoolTexture->getTextureResourceHandle(),
                renderView.depthTargetOverrideLayer,
                1u));
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
            m_pRenderer->cmdSetDepthBias(commandBufferHandle, renderView.shadowBias.depthBiasConstantFactor, renderView.shadowBias.depthBiasClamp, renderView.shadowBias.depthBiasSlopeFactor);

            // ... todo ...

            m_pRenderer->cmdEndRender(commandBufferHandle);
        }
    }

    GraphicsPipelineHandle ShadowPass::resolveGraphicsPipelineHandle(Material* material) const noexcept
    {
        constexpr RenderAttachmentFormats formats{
            .color = DataFormat::Undefined,
            .depth = DataFormat::D32_SFloat
        };

        return material->getGraphicsPipelineHandle(MaterialVariantType::Shadow, formats, true);
    }

    bool ShadowPass::isValidRenderPassMaterial(Material const& material) const noexcept
    {
        return material.getCastsShadows();
    }
}