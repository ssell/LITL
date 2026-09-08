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
            uint32_t firstVertex = 0u;
            uint32_t vertexCount = 0u;
            uint32_t firstIndex = 0u;
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
            // --- Begin rendering

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

            if (compileDrawList(entities))
            {
                // --- Render

                MaterialHandle currMaterialHandle{};
                MeshHandle currMeshHandle{};
                uint32_t currVertexCount = 0u;

                for (auto& drawListItem : drawList)
                {
                    if (!drawListItem.graphicsPipelineHandle.isValid() ||
                         drawListItem.vertexCount == 0u ||
                         drawListItem.indexCount == 0u ||
                         drawListItem.mesh == nullptr || 
                         drawListItem.material == nullptr)
                    {
                        continue;
                    }

                    // --- Material Bind

                    if (drawListItem.materialHandle != currMaterialHandle)
                    {
                        pushConstants.materialPropertiesAddr = 0ull;
                        currMaterialHandle = drawListItem.materialHandle;

                        renderer->cmdBindGraphicsPipeline(frameCommandBuffer, drawListItem.graphicsPipelineHandle);
                        auto pushConstantStages = renderer->getGraphicsPipelinePushConstantStages(drawListItem.graphicsPipelineHandle);

                        if (pushConstantStages != ShaderStage::None)
                        {
                            if (drawListItem.material != nullptr)
                            {
                                if (!drawListItem.material->ready())
                                {
                                    // This material has properties, but the property buffer is not yet ready.
                                    // Likely a material created mid-frame. Skip rendering it for now.
                                    continue;
                                }

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

                    renderer->cmdDrawIndexed(frameCommandBuffer, drawListItem.indexCount, drawListItem.instanceCount, drawListItem.firstIndex, 0, drawListItem.instanceOffset);
                }
            }
            
            // -- End rendering

            renderer->cmdEndRender(frameCommandBuffer);
            renderer->cmdPipelineBarrier(frameCommandBuffer, PipelineBarrierColorToPresent);
            renderer->cmdEnd(frameCommandBuffer);
            renderer->submitCommands(frameCommandBuffer);
        }

        [[nodiscard]] bool compileDrawList(std::vector<RenderableEntity> const& entities) noexcept
        {
            drawList.clear();

            if (entities.empty())
            {
                return false;
            }

            std::optional<MeshHandle> currMeshHandle{ std::nullopt };
            std::optional<MaterialHandle> currMaterialHandle{ std::nullopt };
            //std::optional<MaterialBindingsHandle> currMaterialBindingsHandle{ std::nullopt };

            for (uint32_t i = 0u; i < static_cast<uint32_t>(entities.size()); ++i)
            {
                if ((currMeshHandle != std::nullopt) && (currMeshHandle.value() == entities[i].meshRef.handle) &&
                    (currMaterialHandle != std::nullopt) && (currMaterialHandle.value() == entities[i].materialRef.handle))
                    //(currMaterialBindingsHandle != std::nullopt) && (currMaterialBindingsHandle.value() == entities[i].materialRef.materialBindingsHandle))
                {
                    // Same bound mesh and material(s)
                    continue;
                }

                if (!drawList.empty())
                {
                    drawList.back().instanceCount = i - drawList.back().instanceOffset;
                }

                createDrawListItems(entities[i], i, drawList, currMeshHandle, currMaterialHandle);
            }

            if (drawList.empty())
            {
                return false;
            }

            drawList.back().instanceCount = static_cast<uint32_t>(entities.size()) - drawList.back().instanceOffset;    // Update instance count for the last drawable item
            return true;
        }

        void createDrawListItems(RenderableEntity entity, uint32_t instanceOffset, std::vector<DrawListItem>& drawListItems, std::optional<MeshHandle>& currMeshHandle, std::optional<MaterialHandle>& currMaterialHandle) noexcept
        {
            auto* mesh = objectPool->getMesh(entity.meshRef.handle);

            if (mesh == nullptr)
            {
                return;
            }

            auto& meshDescriptor = mesh->getDescriptor();

            if (entity.firstIndex >= meshDescriptor.indexInfo.indexCount)
            {
                return;
            }

            if (auto* material = objectPool->getMaterial(entity.materialRef.handle); material != nullptr)
            {
                drawListItems.push_back(DrawListItem{
                    .materialHandle = material->getHandle(),
                    .material = material,
                    .graphicsPipelineHandle = material->getGraphicsPipelineHandle(),
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
                currMeshHandle = entity.meshRef.handle;
                currMaterialHandle = entity.materialRef.handle;
            }

            /*
            if (auto* materialBindings = objectPool->getMaterialBindings(entity.materialRef.materialBindingsHandle); materialBindings != nullptr)
            {
                const auto& submeshes = mesh->getGeoMesh().getSubmeshes();

                for (uint32_t i = 0u; (i < static_cast<uint32_t>(submeshes.size())) && (i < materialBindings->getBindingsCount()); ++i)
                {
                    if (auto* material = materialBindings->getBoundMaterial(i); material != nullptr)
                    {
                        drawListItems.push_back(DrawListItem{
                            .materialHandle = material->getHandle(),
                            .material = material,
                            .graphicsPipelineHandle = material->getGraphicsPipelineHandle(),
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
                        currMeshHandle = entity.meshRef.handle;
                        currMaterialBindingsHandle = entity.materialRef.materialBindingsHandle;
                    }
                }
            }
            */
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