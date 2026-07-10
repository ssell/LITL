#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-core/assert.hpp"

namespace litl
{
    void GpuBuffer::create(GpuBufferDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
    }

    void GpuBuffer::destroy() noexcept
    {
        // ... todo? ...
    }

    void GpuBuffer::swapBuffers() noexcept
    {
        // Works for double, triple, etc. incremental buffering.
        m_currentHandle = (m_currentHandle + 1) % m_handles.size();
    }

    void GpuBuffer::swapBuffers(uint32_t frameIndex) noexcept
    {
        LITL_ASSERT_MSG((frameIndex < m_handles.size()), "Requested to buffer swap to invalid index.", );
        m_currentHandle = frameIndex;
    }

    GpuBufferDescriptor const& GpuBuffer::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    BufferHandle GpuBuffer::getBufferHandle() const noexcept
    {
        return m_handles[m_currentHandle];
    }

    void GpuBuffer::setData(std::span<std::byte const> data) noexcept
    {
        m_data.clear();
        m_data.assign(data.begin(), data.end());
    }

    std::span<std::byte const> GpuBuffer::getData() const noexcept
    {
        return m_data;
    }
}