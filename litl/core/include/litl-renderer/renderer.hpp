#ifndef LITL_RENDERER_RENDERER_H__
#define LITL_RENDERER_RENDERER_H__

#include <cstdint>
#include <memory>

#include "litl-renderer/handles.hpp"
#include "litl-renderer/rendererDescriptor.hpp"
#include "litl-renderer/resourceAllocator.hpp"

namespace LITL::Renderer
{
    /// <summary>
    /// Backend implemented renderer operations.
    /// </summary>
    struct RendererOperations
    {
        bool (*build)(RendererHandle const&);
        void (*destroy)(RendererHandle const&);
        uint32_t (*getFrame)(RendererHandle const&);
        uint32_t (*getFrameIndex)(RendererHandle const&);
        void (*render)(RendererHandle const&, CommandBuffer*, uint32_t);
        ResourceAllocator* (*buildResourceAllocator)(RendererHandle const&);
    };

    class Renderer final
    {
    public:

        Renderer(RendererOperations const* pOperations, RendererHandle handle)
            : m_pBackendOperations(pOperations), m_backendHandle(handle)
        {

        }

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const) = delete;

        ~Renderer()
        {
            destroy();
        }

        bool build()
        {
            const auto buildResult = m_pBackendOperations->build(m_backendHandle);

            if (!buildResult)
            {
                return false;
            }

            m_pResourceAllocator.reset(m_pBackendOperations->buildResourceAllocator(m_backendHandle));

            return true;
        }

        void destroy()
        {
            if (m_backendHandle.handle != nullptr)
            {
                m_pBackendOperations->destroy(m_backendHandle);
                m_backendHandle.handle = nullptr;
            }
        }

        uint32_t getFrame() const noexcept
        {
            return m_pBackendOperations->getFrame(m_backendHandle);
        }

        uint32_t getFrameIndex() const noexcept
        {
            return m_pBackendOperations->getFrameIndex(m_backendHandle);
        }

        void render(CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers)
        {
            m_pBackendOperations->render(m_backendHandle, pCommandBuffers, numCommandBuffers);
        }

        RendererHandle const* getHandle() const
        {
            return &m_backendHandle;
        }

        inline ResourceAllocator const* getResourceAllocator() const
        {
            return m_pResourceAllocator.get();
        }

    protected:
        
    private:

        RendererOperations const* m_pBackendOperations;
        RendererHandle m_backendHandle;
        std::unique_ptr<ResourceAllocator> m_pResourceAllocator;
    };
}

#endif