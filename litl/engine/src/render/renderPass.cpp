#include "litl-core/assert.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/render/renderStructs.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-engine/objects/material/material.hpp"
#include "litl-engine/objects/mesh.hpp"

namespace litl
{
    namespace
    {
        static constexpr uint32_t MaxRenderWaitTimeMs = 1000u;
    }

    struct RenderPass::Impl
    {
        struct DrawListItem
        {
            MaterialHandle materialHandle{};
            Material* material{ nullptr };
            GraphicsPipelineHandle graphicsPipelineHandle{};
            MeshHandle meshHandle{};
            Mesh* mesh{};
            uint32_t vertexCount = 0u;
            uint32_t indexCount = 0u;
            uint32_t instanceCount = 0u;
            uint32_t instanceOffset = 0u;
        };

        std::vector<DrawListItem> drawList;
        Renderer* renderer{ nullptr };
        ObjectPool* objectPool{ nullptr };

        void setup(Renderer& renderer, ObjectPool& objectPool) noexcept
        {
            this->renderer = &renderer;
            this->objectPool = &objectPool;
        }

        void render(CommandBufferHandle frameCommandBuffer, RenderPushConstants pushConstants, Camera& camera, std::vector<RenderableEntity> const& entities) noexcept
        {
            // --- Begin renderin

            const BeginRenderCommand beginRenderCommand{
                .color = ColorAttachmentDescriptor { 
                    .colorTexture = {},                     // use the swapchain color texture
                    .clearColor = camera.getClearColor() 
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

            renderer->cmdPipelineBarrier(frameCommandBuffer, PipelineBarrierUndefinedToColor);
            renderer->cmdPipelineBarrier(frameCommandBuffer, PipelineBarrierUndefinedToDepthStencil);
            renderer->cmdBeginRender(frameCommandBuffer, beginRenderCommand);
            renderer->cmdSetViewportAndScissor(frameCommandBuffer, setViewportScissorCommand);

            if (!entities.empty())
            {
                // --- Compile Draw List

                drawList.clear();

                auto currListItem = createDrawListItem(entities[0], 0u);
                drawList.push_back(currListItem);

                for (uint32_t i = 1u; i < static_cast<uint32_t>(entities.size()); ++i)
                {
                    if ((entities[i].material.handle != currListItem.materialHandle) || (entities[i].mesh.handle != currListItem.meshHandle))
                    {
                        drawList.back().instanceCount = i - drawList.back().instanceOffset;
                        currListItem = createDrawListItem(entities[i], i);
                        drawList.push_back(currListItem);
                    }
                }

                drawList.back().instanceCount = static_cast<uint32_t>(entities.size()) - drawList.back().instanceOffset;

                // --- Render
                MaterialHandle currMaterialHandle{};
                MeshHandle currMeshHandle{};
                uint32_t currVertexCount = 0u;

                for (auto& drawListItem : drawList)
                {
                    if (!drawListItem.graphicsPipelineHandle.isValid() ||
                         drawListItem.vertexCount == 0u ||
                         drawListItem.indexCount == 0u)
                    {
                        continue;
                    }

                    // --- Material Bind

                    if (drawListItem.materialHandle != currMaterialHandle)
                    {
                        currMaterialHandle = drawListItem.materialHandle;

                        if (!drawListItem.material->ready())
                        {
                            // This material has properties, but the property buffer is not yet ready.
                            // Likely a material created mid-frame. Skip rendering it for now.
                            continue;
                        }

                        renderer->cmdBindGraphicsPipeline(frameCommandBuffer, drawListItem.graphicsPipelineHandle);
                        auto pushConstantStages = renderer->getGraphicsPipelinePushConstantStages(drawListItem.graphicsPipelineHandle);

                        if (pushConstantStages != ShaderStage::None)
                        {
                            if (drawListItem.material != nullptr)
                            {
                                const auto materialPropsBda = drawListItem.material->getGraphicsBufferDeviceAddress();
                                pushConstants.materialPropertiesAddr = materialPropsBda.has_value() ? materialPropsBda.value() : 0ull;
                            }

                            renderer->cmdPushConstants(
                                frameCommandBuffer,
                                ShaderStage::All,
                                generic_as_byte_span(&pushConstants, sizeof(RenderPushConstants)));
                        }
                    }

                    // --- Mesh Bind

                    if (drawListItem.meshHandle != currMeshHandle)
                    {
                        auto* currVertexBuffer = objectPool->getGpuBuffer(drawListItem.mesh->getVertexBuffer());
                        auto* currIndexBuffer = objectPool->getGpuBuffer(drawListItem.mesh->getIndexBuffer());

                        if ((currVertexBuffer == nullptr) || (currIndexBuffer == nullptr))
                        {
                            // This may be an asset that is still in the process of being loaded in.
                            // Continue here (move to next drawListItem) to skip both the bind and draw.
                            continue;
                        }

                        renderer->cmdBindVertexBuffer(frameCommandBuffer, currVertexBuffer->getBufferHandle(), 0ull, 0u);
                        renderer->cmdBindIndexBuffer(frameCommandBuffer, currIndexBuffer->getBufferHandle(), IndexType::Uint32);  // todo support other index sizes

                        currMeshHandle = drawListItem.meshHandle;
                    }

                    // -- Instanced Draw

                    renderer->cmdDrawIndexed(frameCommandBuffer, drawListItem.indexCount, drawListItem.instanceCount, 0u, 0, drawListItem.instanceOffset);
                }
            }
            
            // -- End rendering

            renderer->cmdEndRender(frameCommandBuffer);
            renderer->cmdPipelineBarrier(frameCommandBuffer, PipelineBarrierColorToPresent);
            renderer->cmdEnd(frameCommandBuffer);
            renderer->submitCommands(frameCommandBuffer);
        }

        DrawListItem createDrawListItem(RenderableEntity entity, uint32_t instanceOffset) noexcept
        {
            auto* material = objectPool->getMaterial(entity.material.handle);
            auto* mesh = objectPool->getMesh(entity.mesh.handle);
            auto& meshDescriptor = mesh->getDescriptor();

            return DrawListItem{
                .materialHandle = entity.material.handle,
                .material = material,
                .graphicsPipelineHandle = material->getGraphicsPipelineHandle(),
                .meshHandle = entity.mesh.handle,
                .mesh = mesh,
                .vertexCount = meshDescriptor.vertexInfo.vertexCount,
                .indexCount = meshDescriptor.indexInfo.indexCount,
                .instanceCount = 0u,
                .instanceOffset = instanceOffset
            };
        }
    };

    RenderPass::RenderPass()
        : m_pImpl(std::make_unique<RenderPass::Impl>())
    {

    }

    RenderPass::~RenderPass()
    {

    }

    void RenderPass::setup(Renderer& renderer, ObjectPool& objectPool) noexcept
    {
        m_pImpl->setup(renderer, objectPool);
    }

    void RenderPass::render(CommandBufferHandle frameCommandBuffer, RenderPushConstants const& pushConstants, Camera& camera, std::vector<RenderableEntity> const& entities) noexcept
    {
        // Note that the conversion from 'RenderPushConstants const&' to 'RendererPushConstants' is intentional so that the pass may mutate the push constants.
        // We keep the 'const&' for this non-pimpl implementation to avoid redundant copies.
        m_pImpl->render(frameCommandBuffer, pushConstants, camera, entities);
    }
}