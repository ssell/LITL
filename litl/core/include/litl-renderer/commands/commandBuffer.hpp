#ifndef LITL_RENDERER_COMMAND_BUFFER_H__
#define LITL_RENDERER_COMMAND_BUFFER_H__

#include <memory>
#include "litl-renderer/handles.hpp"

namespace LITL::Renderer
{
    DEFINE_LITL_HANDLE(CommandBufferHandle);

    struct CommandBufferOperations
    {
        bool (*build)(CommandBufferHandle const&);
        void (*destroy)(CommandBufferHandle const&);
        bool (*begin)(CommandBufferHandle const&, uint32_t);
        bool (*end)(CommandBufferHandle const&);
    };

    class CommandBuffer
    {
    public:

        CommandBuffer(CommandBufferOperations const* pOperations, CommandBufferHandle handle)
            : m_pBackendOperations(pOperations), m_backendHandle(handle)
        {

        }

        CommandBuffer(CommandBuffer const&) = delete;
        CommandBuffer& operator=(CommandBuffer const&) = delete;

        ~CommandBuffer()
        {
            destroy();
        }

        bool build()
        {
            return m_pBackendOperations->build(m_backendHandle);
        }

        void destroy()
        {
            if (m_backendHandle.handle != nullptr)
            {
                m_pBackendOperations->destroy(m_backendHandle);
                m_backendHandle.handle = nullptr;
            }
        }

        bool begin(uint32_t frame)
        {
            return m_pBackendOperations->begin(m_backendHandle, frame);
        }

        bool end()
        {
            return m_pBackendOperations->end(m_backendHandle);
        }

        CommandBufferHandle const* getHandle() const
        {
            return &m_backendHandle;
        }

    protected:

    private:

        CommandBufferHandle m_backendHandle;
        CommandBufferOperations const* m_pBackendOperations;
    };
}

#endif