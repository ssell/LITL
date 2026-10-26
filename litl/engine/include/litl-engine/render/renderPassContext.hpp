#ifndef LITL_ENGINE_RENDER_PASS_CONTEXT_H__
#define LITL_ENGINE_RENDER_PASS_CONTEXT_H__

#include <span>

#include "litl-engine/render/renderStructs.hpp"
#include "litl-engine/render/renderView.hpp"
#include "litl-renderer/resources/commandBuffer.hpp"

namespace litl
{
    struct RenderPassContext
    {
        CommandBufferHandle commandBufferHandle{};
        RenderPushConstants pushConstants{};
        std::span<RenderView const> renderViews{};
    };
}

#endif