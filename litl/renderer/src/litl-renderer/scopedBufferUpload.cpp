#include "litl-renderer/scopedBufferUpload.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    ScopedBufferUpload::ScopedBufferUpload(Renderer const* renderer, CommandBufferHandle commandBufferHandle)
        : m_pRenderer(renderer), m_commandBufferHandle(commandBufferHandle)
    {

    }

    ScopedBufferUpload::~ScopedBufferUpload()
    {
        destroy();
    }

    void ScopedBufferUpload::destroy() noexcept
    {
        if ((m_pRenderer != nullptr) && m_commandBufferHandle.isValid())
        {
            m_pRenderer->cmdBufferFlush(m_commandBufferHandle);
            m_commandBufferHandle = {};
            m_pRenderer = nullptr;
        }
    }
}