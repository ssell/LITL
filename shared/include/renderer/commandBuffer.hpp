#ifndef LITL_COMMAND_BUFFER_H__
#define LITL_COMMAND_BUFFER_H__

#include "renderer/common.hpp"

namespace LITL::Renderer
{
    class CommandBuffer
    {
    public:

        CommandBuffer(RenderContext const* const pRenderContext);
        ~CommandBuffer();

        CommandBuffer(CommandBuffer const& other) = delete;
        CommandBuffer& operator=(CommandBuffer other) = delete;

        CommandBufferContext const* getCommandBufferContext() const;

        /// <summary>
        /// Constructs the Command Buffer if it does not already exist.
        /// </summary>
        /// <returns></returns>
        bool build();

        /// <summary>
        /// Instructs the Command Buffer to begin recording issued commands.
        /// </summary>
        /// <returns></returns>
        bool begin(RenderContext const* const pRenderContext) const;

        /// <summary>
        /// Instructs the Command Buffer to finish recording issued commands.
        /// </summary>
        /// <returns></returns>
        bool end(RenderContext const* const pRenderContext) const;

        /// <summary>
        /// Issues a command to perform a transition of an image from one layout to another, via a pipeline barrier.
        /// </summary>
        /// <param name="pRenderContext"></param>
        /// <param name="imageIndex"></param>
        /// <param name="oldLayout"></param>
        /// <param name="newLayout"></param>
        /// <param name="srcAccessMask"></param>
        /// <param name="dstAccessMask"></param>
        /// <param name="srcStageMask"></param>
        /// <param name="dstStageMask"></param>
        void commandImageLayoutTransition(RenderContext const* const pRenderContext, uint32_t const imageIndex, uint32_t const oldLayout, uint32_t const newLayout, uint32_t const srcAccessMask, uint32_t const dstAccessMask, uint32_t const srcStageMask, uint32_t const dstStageMask) const;

    protected:

    private:

        CommandBufferContext* m_pContext;
    };
}

#endif