#ifndef LITL_ENGINE_RENDER_FORWARD_OPAQUE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_FORWARD_OPAQUE_RENDER_PASS_H__

#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    class ForwardOpaquePass : public RenderPass
    {
    public:

        void render(RenderPassContext const& context) noexcept override;

    protected:

        GraphicsPipelineHandle resolveGraphicsPipelineHandle(Material* material) const noexcept override;

    private:
    };
}

#endif