#ifndef LITL_RENDERER_COMMANDS_SET_VIEWPORT_AND_SCISSOR_H__
#define LITL_RENDERER_COMMANDS_SET_VIEWPORT_AND_SCISSOR_H__

#include <optional>
#include "litl-core/math/types/rect2D.hpp"

namespace litl
{
    /// <summary>
    /// Set the viewport dynamically for a command buffer.
    /// </summary>
    struct SetViewportCommand
    {
        /// <summary>
        /// Normalized offset and dimensions of the viewport region.
        /// </summary>
        rect2D region;

        /// <summary>
        /// Minimum depth value.
        /// </summary>
        float minDepth = 0.0f;

        /// <summary>
        /// Maximum depth value.
        /// </summary>
        float maxDepth = 1.0f;
    };

    struct SetScissorCommand
    {
        /// <summary>
        /// Normalized offset and dimensions of the scissor region.
        /// </summary>
        rect2D region;
    };

    struct SetViewportAndScissorCommand
    {
        std::optional<SetViewportCommand> setViewport;
        std::optional<SetScissorCommand> setScissor;
    };
}

#endif