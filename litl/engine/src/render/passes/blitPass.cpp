#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderPassContext.hpp"
#include "litl-engine/render/passes/blitPass.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-renderer/utility.hpp"

namespace litl
{
    namespace
    {
        struct BlitPushConstants
        {
            uint32_t sourceTextureIndex{ 0u };

            [[nodiscard]] static BlitPushConstants create(Texture* source) noexcept
            {
                BlitPushConstants pushConstants{};

                if (source != nullptr)
                {
                    pushConstants.sourceTextureIndex = packTextureSlotSamplerIndex(
                        source->getTextureTableIndex(), 
                        static_cast<uint32_t>(SamplerPredefines::LinearClampNoMip));
                }

                return pushConstants;
            }
        };
    }

    void BlitPass::setup(ServiceProvider& services, Renderer& renderer, ObjectPool& objectPool) noexcept
    {
        RenderPass::setup(services, renderer, objectPool);
        m_pAssetManager = services.get<AssetManager>().get();
        m_shaderAssetHandle = m_pAssetManager->getShaderHandle("litl/shaders/blit");
    }

    void BlitPass::render(RenderPassContext const& context) noexcept
    {
        // ---------------------------------------------------------------------------------
        // Barrier on each view's source RT

        gatherSourceTexturesIntoScratch(context);

        for (auto& renderViewSource : m_renderViewSourcesScratch)
        {
            if (renderViewSource.color != nullptr)
            {
                m_pRenderer->cmdPipelineBarrier(context.commandBufferHandle, PipelineBarrierCommand::copyWithTexture(PipelineBarrierColorToShaderRead, renderViewSource.color->getTextureResourceHandle()));
            }
        }
        // ---------------------------------------------------------------------------------
        // Barrier on swapchain and then blit each view's source RT onto it

        if (barrierAndBeginRender(context.commandBufferHandle))
        {
            if (ensurePipelineExists())
            {
                for (uint32_t i = 0u; i < static_cast<uint32_t>(context.renderViews.size()); ++i)
                {
                    const RenderView& renderView = context.renderViews[i];
                    const RenderViewSources& renderViewSources = m_renderViewSourcesScratch[i];

                    if (renderViewSources.color != nullptr)
                    {
                        const auto pushConstants = BlitPushConstants::create(renderViewSources.color);

                        m_pRenderer->cmdSetViewportAndScissor(context.commandBufferHandle, createSetViewportAndScissorCommand(renderView.camera, true));
                        m_pRenderer->cmdBindGraphicsPipeline(context.commandBufferHandle, m_graphicsPipelineHandle);
                        m_pRenderer->cmdPushConstants(context.commandBufferHandle, ShaderStage::All, generic_as_byte_span(&pushConstants, sizeof(BlitPushConstants)));
                        m_pRenderer->cmdDraw(context.commandBufferHandle, 3u, 1u, 0u, 0u);
                    }
                }
            }

            m_pRenderer->cmdEndRender(context.commandBufferHandle);
        }
    }

    bool BlitPass::barrierAndBeginRender(CommandBufferHandle commandBufferHandle) noexcept
    {
        const RenderTargetOptions renderTargetOptions{
            .loadOp = LoadOperationType::Clear,
            .colorStoreOp = StoreOperationType::Store,
            .depthStoreOp = StoreOperationType::DontCare,
            .hasColorAttachment = true,
            .hasDepthAttachment = false
        };

        const auto renderTarget = resolveRenderTarget(nullptr, renderTargetOptions);

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

    bool BlitPass::ensurePipelineExists() noexcept
    {
        // We create our own graphics pipeline from a single shader module to avoid limitations of proper Materials.
        // For example, Materials always provide a vertex input layout which we do not use for a full-screen triangle blit.

        if (m_graphicsPipelineHandle.isValid())
        {
            return true;
        }

        auto* shaderAsset = (m_pAssetManager != nullptr) ? m_pAssetManager->getShader(m_shaderAssetHandle) : nullptr;

        if ((shaderAsset == nullptr) || (shaderAsset->shader == nullptr))
        {
            return false;           // Complete asset failure.
        }

        const auto shaderModuleHandle = shaderAsset->shader->getShaderModuleHandle();

        if (!shaderModuleHandle.isValid())
        {
            return false;           // Still loading ...
        }

        const GraphicsPipelineDescriptor graphicsPipelineDescriptor{
            .vertex = PipelineShaderDescriptor {
                .handle = shaderModuleHandle,
                .stage = ShaderStage::Vertex,
                .entryPoint = "vertexMain"
            },
            .fragment = PipelineShaderDescriptor {
                .handle = shaderModuleHandle,
                .stage = ShaderStage::Fragment,
                .entryPoint = "fragmentMain"
            },
            .renderTargets = RenderTargetFormats{
                .colorAttachments = { m_pRenderer->getSwapchainImageFormat() },
                .colorAttachmentCount = 1u
            },                                          // no depth targets for color-only blit
            .vertexInput = {},                          // no vertex bindings or attributes for blit
            .rasterization = RasterizationState{
                .cullMode = CullMode::None 
            },
            .depthStencil = DepthStencilState{
                .depthState = DepthState{
                    .depthTestEnabled = false, 
                    .depthWriteEnabled = false 
                }
            },
            .colorBlend = ColorBlendState{
                .colorAttachmentBlendStates = { ColorBlendAttachmentState{} }
            }
        };

        m_graphicsPipelineHandle = m_pRenderer->createGraphicsPipeline(graphicsPipelineDescriptor);

        return m_graphicsPipelineHandle.isValid();
    }
}