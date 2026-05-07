#ifndef LITL_RENDERER_GPU_BUFFER_H__
#define LITL_RENDERER_GPU_BUFFER_H__

#include <cstdint>

#include "litl-renderer/handles.hpp"

namespace litl
{
    struct GpuBufferOperations
    {
        bool (*build)(GpuBufferHandle const&) noexcept;
        void (*destroy)(GpuBufferHandle const&) noexcept;
        void* (*map)(GpuBufferHandle const&) noexcept;
        void (*unmap)(GpuBufferHandle const&) noexcept;
        uint32_t (*size)(GpuBufferHandle const&) noexcept;
    };

    /// <summary>
    /// Generic GPU buffer.
    /// </summary>
    class GpuBuffer
    {
    public:

        GpuBuffer(GpuBufferOperations const* pOperations, GpuBufferHandle handle)
            : m_pBackendOperations(pOperations), m_backendHandle(handle)
        {

        }

        GpuBuffer(GpuBuffer const&) = delete;
        GpuBuffer& operator=(GpuBuffer const&) = delete;

        ~GpuBuffer()
        {
            destroy();
        }

        [[nodiscard]] bool build() noexcept
        {
            return m_pBackendOperations->build(m_backendHandle);
        }

        void destroy() noexcept
        {
            if ((m_pBackendOperations != nullptr) && (m_backendHandle.handle != nullptr))
            {
                m_pBackendOperations->destroy(m_backendHandle);
                m_backendHandle.handle = nullptr;
            }
        }

        [[nodiscard]] void* map() const noexcept
        {
            return m_pBackendOperations->map(m_backendHandle);
        }

        void unmap() const noexcept
        {
            m_pBackendOperations->unmap(m_backendHandle);
        }

        [[nodiscard]] uint32_t size() const noexcept
        {
            return m_pBackendOperations->size(m_backendHandle);
        }

        [[nodiscard]] GpuBufferHandle const* getHandle() const noexcept
        {
            return &m_backendHandle;
        }

    private:

        GpuBufferOperations const* m_pBackendOperations;
        GpuBufferHandle m_backendHandle;
    };
}

#endif