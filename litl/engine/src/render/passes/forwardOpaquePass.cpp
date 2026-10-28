#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderPassContext.hpp"
#include "litl-engine/render/passes/forwardOpaquePass.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    void ForwardOpaquePass::render(RenderPassContext const& context) noexcept
    {
        RenderPushConstants passPushConstants = context.pushConstants;

        for (const RenderView& renderView : context.renderViews)
        {
            if (renderView.camera == nullptr)
            {
                continue;
            }

            Texture* colorSource{ nullptr };
            Texture* depthSource{ nullptr };

            resolveSourceTextures(renderView, &colorSource, &depthSource);

            if (colorSource != nullptr)
            {
                m_pRenderer->cmdPipelineBarrier(context.commandBufferHandle, PipelineBarrierCommand::copyWithTexture(PipelineBarrierUndefinedToColor, colorSource->getTextureResourceHandle()));
            }
            else
            {
                continue;
            }

            if (depthSource != nullptr)
            {
                m_pRenderer->cmdPipelineBarrier(context.commandBufferHandle, PipelineBarrierCommand::copyWithTexture(PipelineBarrierUndefinedToDepthStencil, depthSource->getTextureResourceHandle()));
            }

            m_pRenderer->cmdBeginRender(context.commandBufferHandle, createBeginRenderCommand(renderView.camera, true, true));
            m_pRenderer->cmdSetViewportAndScissor(context.commandBufferHandle, createSetViewportAndScissorCommand(renderView.camera));

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
                        if ((renderPassItem.material != nullptr) && !renderPassItem.material->ready())
                        {
                            // This material has properties, but the property buffer is not yet ready. Likely a material created mid-frame. Skip rendering it for now.
                            continue;
                        }

                        passPushConstants.materialPropertiesAddr = 0ull;
                        currMaterialHandle = renderPassItem.materialHandle;

                        m_pRenderer->cmdBindGraphicsPipeline(context.commandBufferHandle, renderPassItem.graphicsPipelineHandle);
                        auto pushConstantStages = m_pRenderer->getGraphicsPipelinePushConstantStages(renderPassItem.graphicsPipelineHandle);

                        if (pushConstantStages != ShaderStage::None)
                        {
                            if (renderPassItem.material != nullptr)
                            {
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