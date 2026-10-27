#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderPassContext.hpp"
#include "litl-engine/render/passes/forwardOpaquePass.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    GraphicsPipelineHandle ForwardOpaquePass::resolveGraphicsPipelineHandle(Material* material) const noexcept
    {
        return material->getGraphicsPipelineHandle();
    }

    void ForwardOpaquePass::render(RenderPassContext const& context) noexcept
    {
        RenderPushConstants passPushConstants = context.pushConstants;

        for (const RenderView& renderView : context.renderViews)
        {
            const BeginRenderCommand beginRenderCommand{
                .color = ColorAttachmentDescriptor {
                    .colorTexture = {},                     // use the swapchain color texture
                    .clearColor = renderView.camera->getClearColor()
                },
                .depth = DepthAttachmentDescriptor {
                    .depthTexture = {},                     // use the swapchain depth texture
                    .loadOp = LoadOperationType::Clear,
                    .storeOp = StoreOperationType::DontCare,
                    .clearDepth = 0.0f
                }
            };

            const SetViewportAndScissorCommand setViewportScissorCommand{
                .setViewport = SetViewportCommand {
                    .region = {
                        .offset = { 0.0f, 0.0f },
                        .extents = { 1.0f, 1.0f }           // normalized
                    },
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f
                },
                .setScissor = SetScissorCommand {
                    .region = {
                        .offset = { 0.0f, 0.0f },
                        .extents = { 1.0f, 1.0f }           // normalized
                    }
                }
            };

            m_pRenderer->cmdPipelineBarrier(context.commandBufferHandle, PipelineBarrierUndefinedToColor);
            m_pRenderer->cmdPipelineBarrier(context.commandBufferHandle, PipelineBarrierUndefinedToDepthStencil);
            m_pRenderer->cmdBeginRender(context.commandBufferHandle, beginRenderCommand);
            m_pRenderer->cmdSetViewportAndScissor(context.commandBufferHandle, setViewportScissorCommand);

            if (compileRenderList(renderView.entities))
            {
                // --- Render

                MaterialHandle currMaterialHandle{};
                MeshHandle currMeshHandle{};

                for (auto& renderPassItem : m_renderPassItems)
                {
                    if (!renderPassItem.graphicsPipelineHandle.isValid() ||
                        renderPassItem.vertexCount == 0u ||
                        renderPassItem.indexCount == 0u ||
                        renderPassItem.mesh == nullptr ||
                        renderPassItem.material == nullptr)
                    {
                        continue;
                    }

                    // --- Material Bind

                    if (renderPassItem.materialHandle != currMaterialHandle)
                    {
                        passPushConstants.materialPropertiesAddr = 0ull;
                        currMaterialHandle = renderPassItem.materialHandle;

                        m_pRenderer->cmdBindGraphicsPipeline(context.commandBufferHandle, renderPassItem.graphicsPipelineHandle);
                        auto pushConstantStages = m_pRenderer->getGraphicsPipelinePushConstantStages(renderPassItem.graphicsPipelineHandle);

                        if (pushConstantStages != ShaderStage::None)
                        {
                            if (renderPassItem.material != nullptr)
                            {
                                if (!renderPassItem.material->ready())
                                {
                                    // This material has properties, but the property buffer is not yet ready.
                                    // Likely a material created mid-frame. Skip rendering it for now.
                                    continue;
                                }

                                const auto materialPropsBda = renderPassItem.material->getGraphicsBufferDeviceAddress();
                                passPushConstants.materialPropertiesAddr = materialPropsBda.has_value() ? materialPropsBda.value() : 0ull;
                            }

                            m_pRenderer->cmdPushConstants(
                                context.commandBufferHandle,
                                ShaderStage::All,
                                generic_as_byte_span(&passPushConstants, sizeof(RenderPushConstants)));
                        }
                    }

                    // --- Mesh Bind

                    if (renderPassItem.meshHandle != currMeshHandle)
                    {
                        auto* currVertexBuffer = m_pObjectPool->getGpuBuffer(renderPassItem.mesh->getVertexBuffer());
                        auto* currIndexBuffer = m_pObjectPool->getGpuBuffer(renderPassItem.mesh->getIndexBuffer());

                        if ((currVertexBuffer == nullptr) || (currIndexBuffer == nullptr))
                        {
                            // This may be an asset that is still in the process of being loaded in.
                            // Continue here (move to next renderPassItem) to skip both the bind and draw.
                            continue;
                        }

                        m_pRenderer->cmdBindVertexBuffer(context.commandBufferHandle, currVertexBuffer->getBufferHandle(), 0ull, 0u);
                        m_pRenderer->cmdBindIndexBuffer(context.commandBufferHandle, currIndexBuffer->getBufferHandle(), IndexType::Uint32);  // todo support other index sizes

                        currMeshHandle = renderPassItem.meshHandle;
                    }

                    // -- Instanced Draw

                    m_pRenderer->cmdDrawIndexed(context.commandBufferHandle, renderPassItem.indexCount, renderPassItem.instanceCount, renderPassItem.firstIndex, 0, renderPassItem.instanceOffset);
                }
            }

            m_pRenderer->cmdEndRender(context.commandBufferHandle);
        }
    }
}