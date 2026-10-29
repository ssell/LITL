#ifndef LITL_ENGINE_RENDER_FORWARD_OPAQUE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_FORWARD_OPAQUE_RENDER_PASS_H__

#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    /// <summary>
    /// Responsible for rendering all opaque forward-rendered items.
    /// </summary>
    class ForwardOpaquePass : public RenderPass
    {
    public:

        void render(RenderPassContext const& context) noexcept override;

    private:

        void render(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept;
        [[nodiscard]] bool barrierAndBeginRender(CommandBufferHandle commandBufferHandle, RenderView const& renderView) noexcept;
    };
}

#endif