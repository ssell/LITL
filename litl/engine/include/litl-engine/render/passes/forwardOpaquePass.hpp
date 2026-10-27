#ifndef LITL_ENGINE_RENDER_FORWARD_OPAQUE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_FORWARD_OPAQUE_RENDER_PASS_H__

#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    class ForwardOpaquePass : public RenderPass
    {
    public:

        void setup(Renderer& renderer, ObjectPool& objectPool) noexcept override;
        void render(RenderPassContext& context) noexcept override;

    private:
    };
}

#endif