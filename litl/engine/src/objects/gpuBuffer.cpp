#include "litl-core/assert.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderManager.hpp"

namespace litl
{
    bool GpuBuffer::create(Authority<ObjectPool> auth, GpuBufferDescriptor const& descriptor, RenderManager* renderManager) noexcept
    {
        m_descriptor = descriptor;
        m_pRenderManager = renderManager;
        m_pRenderer = m_pRenderManager->getRenderer();

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

    void GpuBuffer::setBufferHandle(Authority<ObjectPool> auth, GpuBufferHandle handle) noexcept
    {
        m_selfHandle = handle;
    }

    void GpuBuffer::swapBuffers() noexcept
    {
        // Works for double, triple, etc. incremental buffering.
        m_currHandleIndex = (m_currHandleIndex + 1) % m_handles.size();
    }

    void GpuBuffer::swapBuffers(uint32_t frameIndex) noexcept
    {
        LITL_ASSERT_MSG((frameIndex < m_handles.size()), "Requested to buffer swap to invalid index.", );
        m_currHandleIndex = frameIndex;
    }

    GpuBufferDescriptor const& GpuBuffer::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    BufferHandle GpuBuffer::getBufferHandle() const noexcept
    {
        return m_handles[m_currHandleIndex];
    }

    void GpuBuffer::setData(std::span<std::byte const> data) noexcept
    {
        m_data.clear();
        m_data.assign(data.begin(), data.end());
        m_isDirty = true;
        m_pRenderManager->trackDirtyBuffer({}, m_selfHandle);
    }

    void GpuBuffer::setDataImmediate(std::span<std::byte const> data) noexcept
    {
        auto scopedCommandBuffer = m_pRenderer->createScopedCommandBuffer();
        m_pRenderer->cmdBufferUpload(scopedCommandBuffer.get(), data, m_handles[m_currHandleIndex]);
        reset();
    }

    void GpuBuffer::setDataPtr(std::span<std::byte const> data) noexcept
    {
        m_dataPtr = data;
        m_isDirty = true;
        m_pRenderManager->trackDirtyBuffer({}, m_selfHandle);
    }

    std::span<std::byte const> GpuBuffer::getData() const noexcept
    {
        return m_data;
    }

    void GpuBuffer::flushData(Authority<RenderManager> auth, CommandBufferHandle commandBuffer) noexcept
    {
        if (!m_isDirty)
        {
            return;
        }

        auto const currHandle = m_handles[m_currHandleIndex];
        std::span<std::byte const> data = (m_data.empty() ? m_dataPtr : m_data);

        if (!data.empty())
        {
            switch (m_descriptor.memoryUsage)
            {
            case BufferMemoryUsage::ReadBack:       // GPU write, CPU read. Nothing to do.
                break;

            case BufferMemoryUsage::GpuOnly:        // GPU read/write. CPU can't write directly, but can do so via staging buffers.
            case BufferMemoryUsage::Staging:        // GPU read, CPU write.
                m_pRenderer->cmdBufferUpload(commandBuffer, data, currHandle);
                break;

            case BufferMemoryUsage::PersistentMap:  // GPU and CPU read/write.
                {
                    MappedBuffer mappedBuffer{};

                    if (m_pRenderer->mapBuffer(currHandle, mappedBuffer) == RendererResult::Success)
                    {
                        std::memcpy(mappedBuffer.mappedPtr, data.data(), data.size());
                        m_pRenderer->unmapBuffer(currHandle);
                    }

                    break;
                }
            }
        }

        reset();
    }

    void GpuBuffer::reset() noexcept
    {
        m_isDirty = false;
        m_data.clear();             // todo buffer option to maintain CPU data
        m_dataPtr = {};
    }
}