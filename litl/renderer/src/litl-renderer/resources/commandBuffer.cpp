#include "litl-renderer/resources/commandBuffer.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    ScopedCommandBuffer::ScopedCommandBuffer(Renderer const& renderer)
        : m_pRenderer(&renderer)
    {
        m_commandBufferHandle = renderer.createCommandBuffer({ .isTransient = true });
        renderer.cmdBegin(m_commandBufferHandle);
    }

    ScopedCommandBuffer::~ScopedCommandBuffer()
    {
        destroy();
    }

    void ScopedCommandBuffer::destroy() noexcept
    {
        if ((m_pRenderer != nullptr) && m_commandBufferHandle.isValid())
        {
            m_pRenderer->cmdEnd(m_commandBufferHandle);
            m_pRenderer->submitCommandsAndWait(m_commandBufferHandle);
            m_pRenderer->destroyCommandBuffer(m_commandBufferHandle);

            m_commandBufferHandle = {};
        }
    }

    CommandBufferHandle ScopedCommandBuffer::get() const noexcept
    {
        return m_commandBufferHandle;
    }
}