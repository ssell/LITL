#include <optional>

#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    void RenderPass::setup(ServiceProvider& services, RenderManager& renderManager, ObjectPool& objectPool) noexcept
    {
        m_pRenderManager = &renderManager;
        m_pRenderer = renderManager.getRenderer();
        m_pObjectPool = &objectPool;
    }

    void RenderPass::destroy() noexcept
    {

    }

    bool RenderPass::compileRenderList(std::span<RenderableEntity const> entities) noexcept
    {
        m_renderPassItems.clear();

        if (entities.empty())
        {
            return false;
        }

        std::optional<MeshHandle> currMeshHandle{ std::nullopt };
        std::optional<MaterialHandle> currMaterialHandle{ std::nullopt };
        std::optional<uint32_t> currFirstIndex{ std::nullopt };
        std::optional<uint32_t> currIndexCount{ std::nullopt };

        //std::optional<MaterialBindingsHandle> currMaterialBindingsHandle{ std::nullopt };

        for (uint32_t i = 0u; i < static_cast<uint32_t>(entities.size()); ++i)
        {
            auto& renderableEntity = entities[i];

            if (currMeshHandle.has_value() && (*currMeshHandle == renderableEntity.meshRef.handle) &&
                currMaterialHandle.has_value() && (*currMaterialHandle == renderableEntity.materialRef.handle) &&
                currFirstIndex.has_value() && (*currFirstIndex == renderableEntity.firstIndex) &&
                currIndexCount.has_value() && (*currIndexCount == renderableEntity.indexCount))
            {
                // Same bound (sub)mesh and material(s)
                continue;
            }

            if (!m_renderPassItems.empty())
            {
                m_renderPassItems.back().instanceCount = i - m_renderPassItems.back().instanceOffset;
            }

            if (createRenderPassItems(renderableEntity, i, currMeshHandle, currMaterialHandle))
            {
                currMeshHandle = renderableEntity.meshRef.handle;
                currMaterialHandle = renderableEntity.materialRef.handle;
                currFirstIndex = renderableEntity.firstIndex;
                currIndexCount = renderableEntity.indexCount;
            }
        }

        if (m_renderPassItems.empty())
        {
            return false;
        }

        m_renderPassItems.back().instanceCount = static_cast<uint32_t>(entities.size()) - m_renderPassItems.back().instanceOffset;    // Update instance count for the last drawable item

        return true;
    }

    bool RenderPass::createRenderPassItems(RenderableEntity entity, uint32_t instanceOffset, std::optional<MeshHandle>& currMeshHandle, std::optional<MaterialHandle>& currMaterialHandle) noexcept
    {
        auto* mesh = m_pObjectPool->getMesh(entity.meshRef.handle);

        if (mesh == nullptr)
        {
            return false;
        }

        auto& meshDescriptor = mesh->getDescriptor();

        if (entity.firstIndex >= meshDescriptor.indexInfo.indexCount)
        {
            return false;
        }

        auto* material = m_pObjectPool->getMaterial(entity.materialRef.handle);

        if (material == nullptr)
        {
            return false;
        }

        m_renderPassItems.push_back(RenderPassItem{
            .materialHandle = material->getHandle(),
            .material = material,
            .graphicsPipelineHandle = resolveGraphicsPipelineHandle(material),
            .meshHandle = entity.meshRef.handle,
            .mesh = mesh,
            .firstVertex = 0u,
            .vertexCount = meshDescriptor.vertexInfo.vertexCount,
            .firstIndex = entity.firstIndex,
            .indexCount = litl::min(entity.indexCount, meshDescriptor.indexInfo.indexCount - entity.firstIndex),
            .instanceCount = 0u,
            .instanceOffset = instanceOffset
            });

        // Only update current handles on successful object retrievals and subsequent DrawListItem creation.
        return true;
    }

    GraphicsPipelineHandle RenderPass::resolveGraphicsPipelineHandle(Material* material) const noexcept
    {
        return material->getGraphicsPipelineHandle(MaterialVariantType::Color, m_pRenderManager->getSceneRenderFormats(), true);
    }

    SetViewportAndScissorCommand RenderPass::createSetViewportAndScissorCommand(RenderView const& renderView, bool applyCameraViewport) const noexcept
    {
        vec2 viewportOffset{ vec2::zero() };
        vec2 viewportExtents{ vec2::one() };

        if (applyCameraViewport)
        {
            viewportOffset = renderView.view.viewport.offset;
            viewportExtents = renderView.view.viewport.extents;
        }

        const SetViewportAndScissorCommand setViewportScissorCommand{
            .setViewport = SetViewportCommand {
                .region = {
                    .offset = viewportOffset,
                    .extents = viewportExtents
                },
                .minDepth = 0.0f,
                .maxDepth = 1.0f
            },
            .setScissor = SetScissorCommand {
                .region = {
                    .offset = viewportOffset,
                    .extents = viewportExtents
                }
            }
        };

        return setViewportScissorCommand;
    }

    void RenderPass::resolveSourceTextures(RenderView const& renderView, Texture** colorSource, Texture** depthSource) const noexcept
    {
        *colorSource = nullptr;
        *depthSource = nullptr;

        if (renderView.renderTexture == nullptr)
        {
            return;
        }

        *colorSource = m_pObjectPool->getTexture(renderView.renderTexture->getColorTextureHandle());
        *depthSource = m_pObjectPool->getTexture(renderView.renderTexture->getDepthTextureHandle());
    }

    void RenderPass::gatherSourceTexturesIntoScratch(RenderPassContext const& context) noexcept
    {
        m_renderViewSourcesScratch.clear();

        for (const RenderView& renderView : context.renderViews)
        {
            Texture* colorSource{ nullptr };
            Texture* depthSource{ nullptr };

            resolveSourceTextures(renderView, &colorSource, &depthSource);
            m_renderViewSourcesScratch.emplace_back(colorSource, depthSource);
        }
    }

    std::span<PipelineBarrierCommand const> RenderPass::RenderTargetSetup::getPipelineBarrierCmds() const noexcept
    {
        return { pipelineBarrierCmds.data(), barrierCount };
    }

    RenderPass::RenderTargetSetup RenderPass::resolveRenderTarget(RenderView const& renderView, RenderTargetOptions const& options) const noexcept
    {
        RenderTargetSetup setup{};

        auto addAttachment = [&](PipelineBarrierCommand const& preset, TextureResourceHandle handle)
        {
            setup.pipelineBarrierCmds[setup.barrierCount++] = PipelineBarrierCommand::copyWithTexture(preset, handle);
        };

        const bool colorEnabled = options.hasColorAttachment && renderView.view.colorEnabled;
        const bool depthEnabled = options.hasDepthAttachment && renderView.view.depthEnabled;

        Texture* colorTexture{ nullptr };
        Texture* depthTexture{ nullptr };

        if (renderView.renderTexture != nullptr)
        {
            if (colorEnabled)
            {
                colorTexture = m_pObjectPool->getTexture(renderView.renderTexture->getColorTextureHandle());

                if (colorTexture == nullptr)
                {
                    logWarning("Failed to resolve render texture color component.");
                    return setup;
                }
            }

            if (depthEnabled)
            {
                depthTexture = m_pObjectPool->getTexture(renderView.renderTexture->getDepthTextureHandle());

                if ((depthTexture == nullptr) && options.hasDepthAttachment)
                {
                    logWarning("Failed to resolve render texture depth component.");
                    return setup;
                }
            }
        }

        const TextureResourceHandle colorResourceHandle = (colorTexture != nullptr) ? colorTexture->getTextureResourceHandle() : TextureResourceHandle{};
        const TextureResourceHandle depthResourceHandle = (depthTexture != nullptr) ? depthTexture->getTextureResourceHandle() : TextureResourceHandle{};

        if (colorEnabled)
        {
            setup.beginRenderCmd.colorAttachmentType = (colorTexture != nullptr ? RenderAttachmentType::Supplied : RenderAttachmentType::Swapchain);
            setup.beginRenderCmd.colorAttachment = ColorAttachmentDescriptor{
                .colorTexture = colorResourceHandle,
                .loadOp = options.loadOp,
                .storeOp = options.colorStoreOp,
                .clearColor = renderView.view.clearColor
            };

            addAttachment((options.loadOp == LoadOperationType::Load) ? PipelineBarrierColorToColor : PipelineBarrierUndefinedToColor, colorResourceHandle);
        }
        else
        {
            setup.beginRenderCmd.colorAttachmentType = RenderAttachmentType::None;
        }

        if (depthEnabled)
        {
            setup.beginRenderCmd.depthAttachmentType = (depthTexture != nullptr ? RenderAttachmentType::Supplied : RenderAttachmentType::Swapchain);
            setup.beginRenderCmd.depthAttachment = DepthAttachmentDescriptor{
                .depthTexture = depthResourceHandle,
                .loadOp = options.loadOp,
                .storeOp = options.depthStoreOp,
                .clearDepth = 0.0f
            };

            addAttachment((options.loadOp == LoadOperationType::Load) ? PipelineBarrierDepthToDepth : PipelineBarrierUndefinedToDepthStencil, depthResourceHandle);
        }
        else
        {
            setup.beginRenderCmd.depthAttachmentType = RenderAttachmentType::None;
        }

        setup.valid = true;

        return setup;
    }

    void RenderPass::drawRenderList(CommandBufferHandle commandBufferHandle, RenderPushConstants pushConstants, DrawListOptions const& options) noexcept
    {
        GraphicsPipelineHandle boundPipelineHandle{};
        MeshHandle boundMeshHandle{};
        std::optional<uint64_t> pushedMaterialPropertiesAddr{ std::nullopt };

        for (const auto& renderPassItem : m_renderPassItems)
        {
            // --- Validate

            if (!renderPassItem.graphicsPipelineHandle.isValid() ||
                renderPassItem.vertexCount == 0u ||
                renderPassItem.indexCount == 0u ||
                renderPassItem.mesh == nullptr ||
                renderPassItem.material == nullptr)
            {
                continue;
            }

            // --- Resolve Material Properties

            uint64_t materialPropertiesAddr{ 0ull };

            if (options.requiresMaterialProperties)
            {
                if ((renderPassItem.material != nullptr) && !renderPassItem.material->ready())
                {
                    // This material has properties, but the property buffer is not yet ready. Likely a material created mid-frame. Skip rendering it for now.
                    continue;
                }

                materialPropertiesAddr = renderPassItem.material->getGraphicsBufferDeviceAddress().value_or(0ull);
            }

            // --- Resolve Mesh Buffers

            GpuBuffer* vertexBuffer = (renderPassItem.meshHandle != boundMeshHandle) ? m_pObjectPool->getGpuBuffer(renderPassItem.mesh->getVertexBuffer()) : nullptr;
            GpuBuffer* indexBuffer = (renderPassItem.meshHandle != boundMeshHandle) ? m_pObjectPool->getGpuBuffer(renderPassItem.mesh->getIndexBuffer()) : nullptr;

            if ((renderPassItem.meshHandle != boundMeshHandle) && ((vertexBuffer == nullptr) || (indexBuffer == nullptr)))
            {
                // This may be an asset that is still in the process of being loaded in.
                continue;
            }

            // --- Bind

            if (renderPassItem.graphicsPipelineHandle != boundPipelineHandle)
            {
                m_pRenderer->cmdBindGraphicsPipeline(commandBufferHandle, renderPassItem.graphicsPipelineHandle);
                boundPipelineHandle = renderPassItem.graphicsPipelineHandle;
            }

            // Push constants survive pipeline binds because every pipeline layout shares the identical fixed push constant range ({All, 0, 128}). 
            // If that ever changes, this must also re-push on pipeline change.
            if (pushedMaterialPropertiesAddr != materialPropertiesAddr)
            {
                pushConstants.materialPropertiesAddr = materialPropertiesAddr;
                m_pRenderer->cmdPushConstants(commandBufferHandle, ShaderStage::All, generic_as_byte_span(&pushConstants, sizeof(RenderPushConstants)));
                pushedMaterialPropertiesAddr = materialPropertiesAddr;
            }

            if (renderPassItem.meshHandle != boundMeshHandle)
            {
                m_pRenderer->cmdBindVertexBuffer(commandBufferHandle, vertexBuffer->getBufferHandle(), 0ull, 0u);
                m_pRenderer->cmdBindIndexBuffer(commandBufferHandle, indexBuffer->getBufferHandle(), IndexType::Uint32);        // todo support other index sizes
                boundMeshHandle = renderPassItem.meshHandle;
            }

            // --- Instances Draw

            m_pRenderer->cmdDrawIndexed(commandBufferHandle, renderPassItem.indexCount, renderPassItem.instanceCount, renderPassItem.firstIndex, 0, renderPassItem.instanceOffset);
        }
    }

    bool RenderPass::barrierAndBeginRender(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderTargetOptions const& renderTargetOptions) noexcept
    {
        const auto renderTarget = resolveRenderTarget(renderView, renderTargetOptions);

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
}