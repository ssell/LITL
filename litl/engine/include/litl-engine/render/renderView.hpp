#ifndef LITL_ENGINE_RENDER_VIEW_H__
#define LITL_ENGINE_RENDER_VIEW_H__

#include <cstdint>
#include <span>

#include "litl-engine/render/renderableEntity.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-engine/objects/renderTexture.hpp"

namespace litl
{
    enum class RenderViewType : uint8_t
    {
        Camera = 0u,
        Shadow = 1u
    };

    struct RenderView
    {
        Camera* camera{ nullptr };
        RenderTexture* renderTexture{ nullptr };
        std::span<RenderableEntity const> entities{};
        uint64_t perPassDataAddr{ 0ull };
        uint64_t instanceDataAddr{ 0ull };
        RenderViewType type{ RenderViewType::Camera };
    };
}

#endif