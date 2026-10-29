#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    void RenderPass::setup(ServiceProvider& services, Renderer& renderer, ObjectPool& objectPool) noexcept
    {
        m_pRenderer = &renderer;
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
        return material->getGraphicsPipelineHandle();
    }

    BeginRenderCommand RenderPass::createBeginRenderCommand(Camera const* camera, LoadOperationType loadOp, StoreOperationType storeOp, bool allowDepth) const noexcept
    {
        auto colorAttachment = ColorAttachmentDescriptor {
            .colorTexture = {},                                 // If not set, then will use the swapchain.
            .loadOp = loadOp,
            .storeOp = storeOp,
            .clearColor = (camera != nullptr ? camera->getClearColor() : colors::Black)
        };

        auto depthAttachment = DepthAttachmentDescriptor{
            .depthTexture = {},                                 // If not set, then will use the swapchain.
            .loadOp = loadOp,
            .storeOp = storeOp,
            .clearDepth = 0.0f
        };

        if (camera != nullptr)
        {
            allowDepth &= camera->getDepthEnabled();
            RenderTexture* renderTexture = m_pObjectPool->getRenderTexture(camera->getRenderTextureHandle());

            if (renderTexture != nullptr)
            {
                Texture* colorTexture = m_pObjectPool->getTexture(renderTexture->getColorTextureHandle());
                Texture* depthTexture = m_pObjectPool->getTexture(renderTexture->getDepthTextureHandle());

                if (colorTexture != nullptr)
                {
                    colorAttachment.colorTexture = colorTexture->getTextureResourceHandle();
                }

                if (depthTexture != nullptr)
                {
                    depthAttachment.depthTexture = depthTexture->getTextureResourceHandle();
                }
            }
        }

        return BeginRenderCommand{
            .color = colorAttachment,
            .depth = (allowDepth ? std::optional<DepthAttachmentDescriptor>(depthAttachment) : std::nullopt)
        };
    }

    SetViewportAndScissorCommand RenderPass::createSetViewportAndScissorCommand(Camera const* camera, bool applyCameraViewport) const noexcept
    {
        vec2 viewportOffset{ 0.0f, 0.0f };
        vec2 viewportExtents{ 1.0f, 1.0f };

        if (applyCameraViewport && (camera != nullptr))
        {
            const auto& viewportDescriptor = camera->getDescriptor().viewport;
            viewportOffset = viewportDescriptor.offset;
            viewportExtents = viewportDescriptor.extents;
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
                    .offset = { 0.0f, 0.0f },
                    .extents = { 1.0f, 1.0f }           // normalized
                }
            }
        };

        return setViewportScissorCommand;
    }

    void RenderPass::resolveSourceTextures(Camera const* camera, Texture** colorSource, Texture** depthSource) const noexcept
    {
        *colorSource = nullptr;
        *depthSource = nullptr;

        if (camera == nullptr)
        {
            return;
        }

        const RenderTexture* rt = m_pObjectPool->getRenderTexture(camera->getRenderTextureHandle());

        if (rt != nullptr)
        {
            *colorSource = m_pObjectPool->getTexture(rt->getColorTextureHandle());
            *depthSource = m_pObjectPool->getTexture(rt->getDepthTextureHandle());
        }
    }

    void RenderPass::gatherSourceTexturesIntoScratch(RenderPassContext const& context) noexcept
    {
        m_renderViewSourcesScratch.clear();

        for (const RenderView& renderView : context.renderViews)
        {
            Texture* colorSource{ nullptr };
            Texture* depthSource{ nullptr };

            resolveSourceTextures(renderView.camera, &colorSource, &depthSource);
            m_renderViewSourcesScratch.emplace_back(colorSource, depthSource);
        }
    }

    std::span<PipelineBarrierCommand const> RenderPass::RenderTargetSetup::getPipelineBarrierCmds() const noexcept
    {
        return { pipelineBarrierCmds.data(), barrierCount };
    }

    RenderPass::RenderTargetSetup RenderPass::resolveRenderTarget(Camera const* camera, RenderTargetOptions const& options) const noexcept
    {
        RenderTargetSetup setup{};

        auto addAttachment = [&](PipelineBarrierCommand const& preset, TextureResourceHandle handle)
        {
            setup.pipelineBarrierCmds[setup.barrierCount++] = PipelineBarrierCommand::copyWithTexture(preset, handle);
        };

        Texture* color{ nullptr };
        Texture* depth{ nullptr };

        if (camera != nullptr)
        {
            const RenderTexture* renderTexture = m_pObjectPool->getRenderTexture(camera->getRenderTextureHandle());

            if (renderTexture != nullptr)
            {
                color = m_pObjectPool->getTexture(renderTexture->getColorTextureHandle());

                if (color == nullptr)
                {
                    logWarning("Failed to resolve camera render texture color component.");
                    return setup;
                }

                depth = m_pObjectPool->getTexture(renderTexture->getDepthTextureHandle());

                if ((depth == nullptr) && options.wantsDepth)
                {
                    logWarning("Failed to resolve camera render texture depth component.");
                    return setup;
                }
            }
        }

        const TextureResourceHandle colorResourceHandle = (color != nullptr) ? color->getTextureResourceHandle() : TextureResourceHandle{};
        const TextureResourceHandle depthResourceHandle = (depth != nullptr) ? depth->getTextureResourceHandle() : TextureResourceHandle{};

        setup.beginRenderCmd.color = ColorAttachmentDescriptor{
            .colorTexture = colorResourceHandle,
            .loadOp = options.loadOp,
            .storeOp = options.colorStoreOp,
            .clearColor = (camera != nullptr) ? camera->getClearColor() : colors::Black
        };

        addAttachment((options.loadOp == LoadOperationType::Load) ? PipelineBarrierColorToColor : PipelineBarrierUndefinedToColor, colorResourceHandle);

        if (depthResourceHandle.isValid() && options.wantsDepth && ((camera != nullptr) && camera->getDepthEnabled()))
        {
            setup.beginRenderCmd.depth = DepthAttachmentDescriptor{
                .depthTexture = depthResourceHandle,
                .loadOp = options.loadOp,
                .storeOp = options.depthStoreOp,
                .clearDepth = 0.0f
            };

            addAttachment((options.loadOp == LoadOperationType::Load) ? PipelineBarrierDepthToDepth : PipelineBarrierUndefinedToDepthStencil, depthResourceHandle);
        }

        setup.valid = true;

        return setup;
    }
}