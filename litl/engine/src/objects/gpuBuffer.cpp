#include "litl-core/assert.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/objects/gpuBuffer.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderManager.hpp"

namespace litl
{
    bool GpuBuffer::create(Authority<ObjectPool> auth, GpuBufferDescriptor const& descriptor, RenderManager& renderManager) noexcept
    {
        // ---------------------------------------------------------------------------------
        // --- State

        m_descriptor = descriptor;
        m_pRenderManager = &renderManager;
        m_pRenderer = m_pRenderManager->getRenderer();

        if (m_descriptor.itemBytes > 0u)
        {
            if (m_descriptor.bytes < m_descriptor.itemBytes)
            {
                logWarning("GPU Buffer requested with byte size less than that of a single item byte size.");
            }
            else if ((m_descriptor.bytes % m_descriptor.itemBytes) != 0u)
            {
                logWarning("GPU Buffer requested with byte size not equal to an evenly divisible number of item bytes.");
            }
        }

        // ---------------------------------------------------------------------------------
        // --- Handles

        switch (m_descriptor.bufferStrategy)
        {
        case GpuBufferingStrategy::Single:
            m_buffers.resize(1ull);
            break;

        case GpuBufferingStrategy::Double:
            m_buffers.resize(2ull);
            break;

        case GpuBufferingStrategy::Frame:
            m_buffers.resize(m_pRenderer->getFrameData().framesInFlight);
            break;
        }

        if (has_any(descriptor.type, BufferTypeFlagBits::BufferDeviceAddress))
        {
            m_usesBDA = true;
            m_bdaAddresses.resize(m_buffers.size());
        }

        for (auto i = 0; i < m_buffers.size(); ++i)
        {
            createBuffer(i);
        }

        // ---------------------------------------------------------------------------------
        // --- Buffer Device Addresses

        return true;
    }

    void GpuBuffer::createBuffer(uint32_t index) noexcept
    {
        LITL_ASSERT_MSG((index < m_buffers.size()), "Index into GPU Buffer internal buffer is out-of-range.", );

        if (m_buffers[index].version == m_version)
        {
            // Buffer is already up-to-date. No action.
            return;
        }

        const auto bufferDescriptor = BufferDescriptor{
            .type = m_descriptor.type,
            .memory = BufferMemoryType::Auto,
            .memoryUsage = m_descriptor.memoryUsage,
            .sharing = SharingMode::Exclusive,
            .bytes = m_descriptor.bytes
        };

        bool bufferUpdated = false;

        if (m_buffers[index].version == 0u)
        {
            // First creation of the buffer.
            m_buffers[index].handle = m_pRenderer->createBuffer(bufferDescriptor);
            bufferUpdated = true;
        }
        else
        {
            if (m_descriptor.canResize)
            {
                // Resizing of the buffer. Need to transfer from the old buffer.
                auto oldBufferHandle = m_buffers[index].handle;
                auto newBufferHandle = m_pRenderer->createBuffer(bufferDescriptor);

                // ... todo copy data from old to new ...

                m_buffers[index].handle = newBufferHandle;
                bufferUpdated = true;

                m_pRenderer->destroyBuffer(oldBufferHandle);
            }
        }

        if (bufferUpdated)
        {
            // Update to match the current version of the GPU Buffer wrapper
            m_buffers[index].version = m_version;

            if (m_usesBDA)
            {
                // Update the Buffer Device Address
                auto bda = m_pRenderer->getBufferDeviceAddress(m_buffers[index].handle);
                LITL_ASSERT_MSG(bda.has_value(), "Failed to retrieve BDA for buffer created with BufferDeviceAddress flag.", );
                m_bdaAddresses[index] = *bda;
            }
        }
    }

    void GpuBuffer::destroy(Authority<ObjectPool> auth) noexcept
    {
        for (auto& versionedBuffer : m_buffers)
        {
            m_pRenderer->destroyBuffer(versionedBuffer.handle);
        }
    }

    void GpuBuffer::setBufferHandle(Authority<ObjectPool> auth, GpuBufferHandle handle) noexcept
    {
        m_selfHandle = handle;
    }

    void GpuBuffer::swapBuffers() noexcept
    {
        // Works for double, triple, etc. incremental buffering.
        m_currHandleIndex = (m_currHandleIndex + 1) % m_buffers.size();

        if (m_buffers[m_currHandleIndex].version < m_version)
        {
            createBuffer(m_currHandleIndex);
        }
    }

    void GpuBuffer::swapBuffers(uint32_t frameIndex) noexcept
    {
        LITL_ASSERT_MSG((frameIndex < m_buffers.size()), "Requested swap GPU Buffer internal handle to invalid index.", );
        m_currHandleIndex = frameIndex;
    }

    GpuBufferDescriptor const& GpuBuffer::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    BufferHandle GpuBuffer::getBufferHandle() const noexcept
    {
        return m_buffers[m_currHandleIndex].handle;
    }

    std::optional<uint64_t> GpuBuffer::getBufferDeviceAddress() const noexcept
    {
        if (m_bdaAddresses.empty())
        {
            return std::nullopt;
        }

        LITL_ASSERT_MSG((m_currHandleIndex < m_bdaAddresses.size()), "Request for GPU Buffer BDA is out-of-range.", std::nullopt);
        return m_bdaAddresses[m_currHandleIndex];
    }

    void GpuBuffer::setData(std::span<std::byte const> data) noexcept
    {
        m_data.clear();
        m_data.assign(data.begin(), data.end());
        m_isDirty = true;
        m_pRenderManager->trackDirtyBuffer({}, m_selfHandle);
    }

    void GpuBuffer::setDataImmediate(std::span<std::byte const> data, std::optional<CommandBufferHandle> commandBuffer) noexcept
    {
        m_dataPtr = data;
        m_isDirty = true;

        if (commandBuffer.has_value() && commandBuffer.value().isValid())
        {
            flushData(commandBuffer.value());
        }
        else
        {
            ScopedCommandBuffer scopedCommandBuffer = m_pRenderer->createScopedCommandBuffer();
            flushData(scopedCommandBuffer.get());
        }
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
        flushData(commandBuffer);
    }

    void GpuBuffer::flushData(CommandBufferHandle commandBuffer) noexcept
    {
        if (!m_isDirty)
        {
            return;
        }

        auto const currHandle = m_buffers[m_currHandleIndex].handle;
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

    uint32_t GpuBuffer::getSizeBytes() const noexcept
    {
        return m_descriptor.bytes;
    }

    uint32_t GpuBuffer::getItemCapacity() const noexcept
    {
        if (m_descriptor.itemBytes == 0u)
        {
            return 0u;
        }

        return (m_descriptor.bytes / m_descriptor.itemBytes);
    }

    void GpuBuffer::resizeBytes(uint32_t size, bool canShrink, bool immediate) noexcept
    {
        LITL_ASSERT_MSG((m_descriptor.canResize && (size > 0u)), "Invalid buffer resize requested.", );

        if ((size < m_descriptor.bytes) && !canShrink)
        {
            // Requested a smaller buffer, but did not provide permission to shrink.
            return;
        }

        m_version++;

        if (immediate)
        {
            // Update the current buffer. Other buffers will be updated when they are swapped to.
            createBuffer(m_currHandleIndex);
        }
    }

    void GpuBuffer::resizeItems(uint32_t items, bool canShrink, bool immediate) noexcept
    {
        resizeBytes(items * m_descriptor.itemBytes);
    }
}