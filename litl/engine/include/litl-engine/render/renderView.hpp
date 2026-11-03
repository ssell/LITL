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
        DepthOnly = 1u,
        Shadow = 2u
    };

    struct RenderView
    {
        Camera* camera{ nullptr };
        RenderTexture* renderTexture{ nullptr };
        std::span<RenderableEntity const> sortedOpaqueEntities{};
        std::span<RenderableEntity const> sortedTransparentEntities{};
        uint64_t perPassDataAddr{ 0ull };
        uint64_t opaqueInstanceDataAddr{ 0ull };
        uint64_t transparentInstanceDataAddr{ 0ull };
        RenderViewType type{ RenderViewType::Camera };

        [[nodiscard]] static RenderViewType resolveType(Camera* camera) noexcept
        {
            if (camera != nullptr)
            {
                switch (camera->getCameraType())
                {
                case CameraType::DepthOnly:
                    return RenderViewType::DepthOnly;

                case CameraType::Standard:
                default:
                    return RenderViewType::Camera;
                }
            }
            else
            {
                return RenderViewType::Shadow;
            }
        }
    };
}

#endif