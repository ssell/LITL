#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-core/assert.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    bool GpuBuffer::create(Authority<ObjectPool> auth, GpuBufferDescriptor const& descriptor, Renderer const* renderer) noexcept
    {
        m_descriptor = descriptor;
        m_pRenderer = renderer;

        switch (m_descriptor.bufferStrategy)
        {
        case GpuBufferingStrategy::Single:
            m_handles.resize(1);
            break;

        case GpuBufferingStrategy::Double:
            m_handles.resize(2);
            break;

        case GpuBufferingStrategy::Frame:
            m_handles.resize(m_pRenderer->getFrameData().framesInFlight);
            break;
        }

        const auto bufferDescriptor = BufferDescriptor{
                .type = descriptor.type,
                .memory = BufferMemoryType::Auto,
                .memoryUsage = descriptor.memoryUsage,
                .sharing = SharingMode::Exclusive,
                .bytes = descriptor.bytes
        };

        for (auto i = 0; i < m_handles.size(); ++i)
        {
            m_handles[i] = m_pRenderer->createBuffer(bufferDescriptor);
            LITL_ASSERT_MSG(m_handles[i].isValid(), "Failed to create GPU Buffer", false);
        }

        return true;
    }

    void GpuBuffer::destroy(Authority<ObjectPool> auth) noexcept
    {
        for (auto& bufferHandle : m_handles)
        {
            m_pRenderer->destroyBuffer(bufferHandle);
        }
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