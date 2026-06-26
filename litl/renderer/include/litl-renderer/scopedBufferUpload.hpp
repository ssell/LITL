#ifndef LITL_RENDERER_SCOPED_BUFFER_UPLOAD_H__
#define LITL_RENDERER_SCOPED_BUFFER_UPLOAD_H__

#include "litl-renderer/resources/commandBuffer.hpp"

namespace litl
{
    class Renderer;

    /// <summary>
    /// Utility helper class that calls cmdBufferFlush on destruction.
    /// </summary>
    class ScopedBufferUpload
    {
    public:

        explicit ScopedBufferUpload(Renderer const* renderer, CommandBufferHandle commandBufferHandle);
        ~ScopedBufferUpload();
        void destroy() noexcept;

    private:

        Renderer const* m_pRenderer;
        CommandBufferHandle m_commandBufferHandle;
    };
}

#endif