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
            render(context.commandBufferHandle, renderView, context.pushConstants);
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

                        viewPushConstants.materialPropertiesAddr = 0ull;
                        currMaterialHandle = renderPassItem.materialHandle;

                        m_pRenderer->cmdBindGraphicsPipeline(commandBufferHandle, renderPassItem.graphicsPipelineHandle);
                        auto pushConstantStages = m_pRenderer->getGraphicsPipelinePushConstantStages(renderPassItem.graphicsPipelineHandle);

                        if (pushConstantStages != ShaderStage::None)
                        {
                            if (renderPassItem.material != nullptr)
                            {
                                const auto materialPropsBda = renderPassItem.material->getGraphicsBufferDeviceAddress();
                                viewPushConstants.materialPropertiesAddr = materialPropsBda.has_value() ? materialPropsBda.value() : 0ull;
                            }

                            m_pRenderer->cmdPushConstants(
                                commandBufferHandle,
                                ShaderStage::All,
                                generic_as_byte_span(&viewPushConstants, sizeof(RenderPushConstants)));
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

                        m_pRenderer->cmdBindVertexBuffer(commandBufferHandle, currVertexBuffer->getBufferHandle(), 0ull, 0u);
                        m_pRenderer->cmdBindIndexBuffer(commandBufferHandle, currIndexBuffer->getBufferHandle(), IndexType::Uint32);  // todo support other index sizes

                        currMeshHandle = renderPassItem.meshHandle;
                    }

                    // -- Instanced Draw

                    m_pRenderer->cmdDrawIndexed(commandBufferHandle, renderPassItem.indexCount, renderPassItem.instanceCount, renderPassItem.firstIndex, 0, renderPassItem.instanceOffset);
                }
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

        const auto renderTarget = resolveRenderTarget(renderView.camera, renderTargetOptions);

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