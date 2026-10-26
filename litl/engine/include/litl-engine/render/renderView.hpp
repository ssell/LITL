#ifndef LITL_ENGINE_RENDER_VIEW_H__
#define LITL_ENGINE_RENDER_VIEW_H__

#include <cstdint>
#include <span>

#include "litl-engine/render/renderableEntity.hpp"
#include "litl-engine/objects/camera.hpp"

namespace litl
{
    struct RenderView
    {
        const Camera* camera{ nullptr };
        std::span<RenderableEntity const> entities{};
        uint64_t perPassDataAddr{ 0ull };
        uint64_t instanceDataAddr{ 0ull };
    };
}

#endif