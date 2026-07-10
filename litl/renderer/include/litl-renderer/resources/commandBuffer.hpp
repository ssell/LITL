#ifndef LITL_RENDERER_COMMAND_BUFFER_H__
#define LITL_RENDERER_COMMAND_BUFFER_H__

#include "litl-core/handles.hpp"

namespace litl
{
    class Renderer;

    struct CommandBufferDescriptor
    {
        /// <summary>
        /// Is this a short-lived buffer used for a single submission?
        /// </summary>
        bool isTransient = false;
    };

    struct CommandBufferTag {};
    using CommandBufferHandle = Handle<CommandBufferTag>;

    /// <summary>
    /// A transient command buffer that submits its command when it is destroyed/leaves scope.
    /// </summary>
    class ScopedCommandBuffer
    {
    public:

        ScopedCommandBuffer(Renderer const& renderer);
        ~ScopedCommandBuffer();

        ScopedCommandBuffer(ScopedCommandBuffer const&) = delete;
        ScopedCommandBuffer& operator=(ScopedCommandBuffer const&) = delete;

        /// <summary>
        /// Submits the transient command buffer and then destroys it.
        /// This will be called automatically by the destructor and does not need to be manually called.
        /// </summary>
        void destroy() noexcept;

        /// <summary>
        /// Retrieves the CommandBufferHandle held by this scoped buffer.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]]CommandBufferHandle get() const noexcept;

    private:

        Renderer const* m_pRenderer = nullptr;
        CommandBufferHandle m_commandBufferHandle{};

    };
}

#endif