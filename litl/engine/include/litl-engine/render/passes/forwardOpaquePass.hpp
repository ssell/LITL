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
    };
}

#endif