#ifndef LITL_ENGINE_RENDER_VIEW_H__
#define LITL_ENGINE_RENDER_VIEW_H__

#include <cstdint>
#include <optional>
#include <span>

#include "litl-core/math/types.hpp"
#include "litl-core/math/bounds.hpp"
#include "litl-engine/render/renderableEntity.hpp"
#include "litl-engine/objects/renderTexture.hpp"

namespace litl
{
    class Camera;

    enum class RenderViewType : uint8_t
    {
        Camera = 0u,
        DepthOnly = 1u,
        Shadow = 2u
    };

    struct ResolvedView
    {
        mat4 viewMat{};
        mat4 projMat{};
        mat4 viewProjMat{};
        vec3 position{};
        rect2D viewport{ .offset = vec2::zero(), .extents = vec2::one() };
        bounds::Frustum frustum{};
        bool colorEnabled{ true };
        bool depthEnabled{ false };
        color clearColor{ colors::Black };
        float near{ 0.0f };
        float far{ 1.0f };

        [[nodiscard]] static ResolvedView fromCamera(Camera const& camera) noexcept;
    };

    struct RenderView
    {
        ResolvedView view{};
        RenderTexture* renderTexture{ nullptr };
        std::span<RenderableEntity const> sortedOpaqueEntities{};
        std::span<RenderableEntity const> sortedTransparentEntities{};
        uint64_t perPassDataAddr{ 0ull };
        uint64_t opaqueInstanceDataAddr{ 0ull };
        uint64_t transparentInstanceDataAddr{ 0ull };
        RenderViewType type{ RenderViewType::Camera };

        [[nodiscard]] static RenderViewType resolveType(Camera* camera) noexcept;
    };
}

#endif