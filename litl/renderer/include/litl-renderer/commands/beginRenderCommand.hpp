#ifndef LITL_RENDERER_COMMANDS_BEGIN_RENDER_H__
#define LITL_RENDERER_COMMANDS_BEGIN_RENDER_H__

#include <optional>
#include <vector>

#include "litl-core/math/types.hpp"
#include "litl-renderer/resources/texture.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct ColorAttachmentDescriptor
    {
        /// <summary>
        /// The color texture. If not specified (left at default value), then the current swapchain image will be used.
        /// </summary>
        TextureHandle colorTexture{};

        /// <summary>
        /// How are the contents treated at the beginning of the pass?
        /// </summary>
        LoadOperationType loadOp = LoadOperationType::Load;

        /// <summary>
        /// How are the contents treated at the end of the pass?
        /// </summary>
        StoreOperationType storeOp = StoreOperationType::Store;

        /// <summary>
        /// The color value to clear to.
        /// </summary>
        color clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

        /// <summary>
        /// Optional resolve texture (MSAA)
        /// </summary>
        TextureHandle resolveTexture{};
    };

    struct DepthAttachmentDescriptor
    {
        /// <summary>
        /// The depth texture.
        /// </summary>
        TextureHandle depthTexture{};

        /// <summary>
        /// How are the contents treated at the beginning of the pass?
        /// </summary>
        LoadOperationType loadOp = LoadOperationType::Load;

        /// <summary>
        /// How are the contents treated at the end of the pass?
        /// </summary>
        StoreOperationType storeOp = StoreOperationType::Store;

        /// <summary>
        /// The depth value to clear to.
        /// </summary>
        float clearDepth = 1.0f;

        /// <summary>
        /// The stencil value to clear to.
        /// </summary>
        uint32_t clearStencil = 0u;
    };

    struct BeginRenderCommand
    {
        /// <summary>
        /// The color texture to render to.
        /// If not specified, will default to the current swapchain image.
        /// </summary>
        ColorAttachmentDescriptor color;
        // ^ todo update to allow for multiple?

        /// <summary>
        /// 
        /// </summary>
        std::optional<DepthAttachmentDescriptor> depth;

        /// <summary>
        /// 
        /// </summary>
        rect2D area = { .offset = { 0.0f, 0.0f }, .extents = { 0.0f, 0.0f } };

        /// <summary>
        /// 
        /// </summary>
        uint32_t layerCount = 1u;

        /// <summary>
        /// 
        /// </summary>
        uint32_t viewMask = 0u;
    };
}

#endif