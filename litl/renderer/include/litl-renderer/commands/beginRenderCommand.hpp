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
        TextureResourceHandle colorTexture{};

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
        TextureResourceHandle resolveTexture{};
    };

    struct DepthAttachmentDescriptor
    {
        /// <summary>
        /// The depth texture. If not specified (left at default value), then the current swapchain depth texture will be used.
        /// </summary>
        TextureResourceHandle depthTexture{};

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
        /// Note we use a reversed depth buffer for the main camera, so the far plane sits at 0.
        /// </summary>
        float clearDepth = 0.0f;

        /// <summary>
        /// The stencil value to clear to.
        /// </summary>
        uint32_t clearStencil = 0u;
    };

    /// <summary>
    /// The type of color/depth attachment specified in the command.
    /// </summary>
    enum class RenderAttachmentType : uint8_t
    {
        /// <summary>
        /// Indicates that this attachment type is not used.
        /// </summary>
        None = 0u,

        /// <summary>
        /// Indicates that this attachment type is used and is provided.
        /// </summary>
        Supplied = 1u,

        /// <summary>
        /// Indicates that this attachment type is used and is the swapchain (not provided directly by the user).
        /// </summary>
        Swapchain = 2u
    };

    struct BeginRenderCommand
    {
        RenderAttachmentType colorAttachmentType{ RenderAttachmentType::Swapchain };
        std::optional<ColorAttachmentDescriptor> colorAttachment{ std::nullopt };

        RenderAttachmentType depthAttachmentType{ RenderAttachmentType::None };
        std::optional<DepthAttachmentDescriptor> depthAttachment{ std::nullopt };

        /// <summary>
        /// Both .offset and .extents are normalized coordinates within the target texture.
        /// </summary>
        rect2D area = { .offset = { 0.0f, 0.0f }, .extents = { 1.0f, 1.0f } };
        uint32_t layerCount = 1u;
        uint32_t viewMask = 0;
    };
}

#endif