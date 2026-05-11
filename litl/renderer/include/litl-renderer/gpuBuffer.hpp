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
        void (*write)(GpuBufferHandle const&, void*, uint32_t, uint32_t) noexcept;
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

        /// <summary>
        /// Writes the data, with size in bytes, into the buffer at the specified offset.
        /// </summary>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="size"></param>
        void write(void* data, uint32_t offset, uint32_t size) noexcept
        {
            m_pBackendOperations->write(m_backendHandle, data, offset, size);
        }

        /// <summary>
        /// Returns the size of the buffer in bytes.
        /// </summary>
        /// <returns></returns>
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