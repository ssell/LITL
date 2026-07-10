#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/objects/camera.hpp"

namespace litl
{
    namespace
    {
        static constexpr uint32_t MaxRenderWaitTimeMs = 1000u;
    }

    void RenderPass::render(Renderer* renderer, Camera* camera, std::vector<RenderableEntity> const& entities) const noexcept
    {
        if (!camera->isMainCamera())
        {
            // ... only main camera for now just to get things working ...
            return;
        }

        auto commandBuffer = renderer->cmdBeginFrame();

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

        renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierUndefinedToColor);
        renderer->cmdBeginRender(commandBuffer, beginRenderCommand);
        renderer->cmdSetViewportAndScissor(commandBuffer, setViewportScissorCommand);

        // --- Render

        if (!entities.empty())
        {
            // ... todo ...
        }

        // -- End rendering

        renderer->cmdEndRender(commandBuffer);
        renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierColorToPresent);
        renderer->cmdEnd(commandBuffer);
        renderer->submitCommands(commandBuffer);
    }
}