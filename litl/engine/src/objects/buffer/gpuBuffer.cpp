#include "litl-engine/objects/buffer/gpuBuffer.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-core/assert.hpp"

namespace litl
{
    void GpuBuffer::create(GpuBufferDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
    }

    GpuBufferDescriptor const& GpuBuffer::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    BufferHandle GpuBuffer::getBufferHandle() const noexcept
    {
        return m_handle;
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