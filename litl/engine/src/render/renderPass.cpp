#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-engine/objects/material.hpp"
#include "litl-engine/objects/mesh.hpp"

namespace litl
{
    namespace
    {
        static constexpr uint32_t MaxRenderWaitTimeMs = 1000u;
    }

    void RenderPass::render(
        Renderer const& renderer, 
        CommandBufferHandle frameCommandBuffer, 
        ObjectPool& objectPool, 
        Camera* camera, 
        std::vector<RenderableEntity> const& entities) const noexcept
    {
        if (!camera->isMainCamera())
        {
            // ... only main camera for now just to get things working ...
            return;
        }

        // --- Begin rendering

        const BeginRenderCommand beginRenderCommand{
            .color = ColorAttachmentDescriptor {
                .clearColor = color(0.05f, 0.05f, 0.075f, 1.0f)
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

        renderer.cmdPipelineBarrier(frameCommandBuffer, PipelineBarrierUndefinedToColor);
        renderer.cmdBeginRender(frameCommandBuffer, beginRenderCommand);
        renderer.cmdSetViewportAndScissor(frameCommandBuffer, setViewportScissorCommand);

        // --- Render

        if (!entities.empty())
        {
            // Handles are kept to speed up change in material/mesh checks. Otherwise would have to retrieve from the object pool each time.

            MaterialHandle currMaterialHandle{};
            MeshHandle currMeshHandle{};
            uint32_t currVertexCount = 0u;

            for (auto& entity : entities)
            {
                if (entity.material.handle != currMaterialHandle)
                {
                    currMaterialHandle = entity.material.handle;
                    auto* currMaterial = objectPool.getMaterial(currMaterialHandle);

                    // ... todo bind material ...
                }

                if (entity.mesh.handle != currMeshHandle)
                {
                    currMeshHandle = entity.mesh.handle;
                    auto* currMesh = objectPool.getMesh(currMeshHandle);
                    auto* currVertexBuffer = objectPool.getGpuBuffer(currMesh->getVertexBuffer());
                    auto* currIndexBuffer = objectPool.getGpuBuffer(currMesh->getIndexBuffer());
                    currVertexCount = currMesh->getDescriptor().vertexInfo.vertexCount;                                     // todo this only works for static sized buffers

                    renderer.cmdBindVertexBuffer(frameCommandBuffer, currVertexBuffer->getBufferHandle(), 0ull, 0u);
                    renderer.cmdBindIndexBuffer(frameCommandBuffer, currIndexBuffer->getBufferHandle(), IndexType::Uint32);  // todo support other index sizes
                }

                renderer.cmdDraw(frameCommandBuffer, currVertexCount, 1u, 0u, 0u);
            }
        }

        // -- End rendering

        renderer.cmdEndRender(frameCommandBuffer);
        renderer.cmdPipelineBarrier(frameCommandBuffer, PipelineBarrierColorToPresent);
        renderer.cmdEnd(frameCommandBuffer);
        renderer.submitCommands(frameCommandBuffer);
    }
}