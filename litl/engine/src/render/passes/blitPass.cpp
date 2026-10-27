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

        void cmdPipelineBarriers(CommandBufferHandle commandBuffer, Renderer* renderer, Texture* colorSource)
        {
            // Barrier to ensure the source color texture that we are blitting is ready
            if (colorSource != nullptr)
            {
                renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierCommand::copyWithTexture(PipelineBarrierColorToShaderRead, colorSource->getTextureResourceHandle()));
            }

            // Barrier to ensure the swapchain is ready. Using the default PipelineBarrierUndefinedToColor without a texture set uses the swapchain.
            renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierUndefinedToColor);
        }
    }

    void BlitPass::setup(ServiceProvider& services, Renderer& renderer, ObjectPool& objectPool) noexcept
    {
        RenderPass::setup(services, renderer, objectPool);
        m_pAssetManager = services.get<AssetManager>().get();
    }

    void BlitPass::render(RenderPassContext const& context) noexcept
    {
        Texture* colorSource{ nullptr };
        Texture* depthSource{ nullptr };

        resolveSourceTextures(context, colorSource, depthSource);
        cmdPipelineBarriers(context.commandBufferHandle, m_pRenderer, colorSource);

        m_pRenderer->cmdBeginRender(context.commandBufferHandle, createBeginRenderCommand(nullptr, false));

        if ((colorSource != nullptr) && ensurePipelineExists())
        {
            const auto pushConstants = BlitPushConstants::create(colorSource);

            m_pRenderer->cmdSetViewportAndScissor(context.commandBufferHandle, createSetViewportAndScissorCommand(nullptr));
            m_pRenderer->cmdBindGraphicsPipeline(context.commandBufferHandle, m_graphicsPipelineHandle);
            m_pRenderer->cmdPushConstants(context.commandBufferHandle, m_pushConstantStages, generic_as_byte_span(&pushConstants, sizeof(BlitPushConstants)));
            m_pRenderer->cmdDraw(context.commandBufferHandle, 3u, 1u, 0u, 0u);
        }

        m_pRenderer->cmdEndRender(context.commandBufferHandle);
    }

    bool BlitPass::ensurePipelineExists() noexcept
    {
        if (m_graphicsPipelineHandle.isValid())
        {
            return true;
        }

        auto* shaderAsset = (m_pAssetManager != nullptr) ? m_pAssetManager->getShader(m_shaderAssetHandle) : nullptr;

        if (shaderAsset == nullptr)
        {
            return false;           // Still loading ...
        }

        const auto shaderModuleHandle = shaderAsset->shader->getShaderModuleHandle();

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
        m_pushConstantStages = m_pRenderer->getGraphicsPipelinePushConstantStages(m_graphicsPipelineHandle);

        return m_graphicsPipelineHandle.isValid();
    }
}