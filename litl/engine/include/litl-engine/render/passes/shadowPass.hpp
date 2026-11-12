#ifndef LITL_ENGINE_RENDER_SHADOW_PASS_H__
#define LITL_ENGINE_RENDER_SHADOW_PASS_H__

#include "litl-engine/render/renderPass.hpp"

namespace litl
{
    class ShadowPass : public RenderPass
    {
    public:

        void render(RenderPassContext const& context) noexcept override;

    private:

        void render(CommandBufferHandle commandBufferHandle, RenderView const& renderView, RenderPushConstants viewPushConstants) noexcept;
    };
}

#endif