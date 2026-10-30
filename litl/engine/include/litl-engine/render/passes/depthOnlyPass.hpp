#ifndef LITL_ENGINE_RENDER_DEPTH_ONLY_PASS_H__
#define LITL_ENGINE_RENDER_DEPTH_ONLY_PASS_H__

#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    class DepthOnlyPass : public RenderPass
    {
    public:

        void render(RenderPassContext const& context) noexcept override;

    protected:

        [[nodiscard]] GraphicsPipelineHandle resolveGraphicsPipelineHandle(Material* material) const override;
    };
}

#endif