#ifndef LITL_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_RENDERER_PIPELINE_LAYOUT_H__

#include <memory>
#include "litl-renderer/handles.hpp"

namespace LITL::Renderer
{
    DEFINE_LITL_HANDLE(PipelineLayoutHandle);

    /// <summary>
    /// Configuration for the pipeline layout.
    /// </summary>
    struct PipelineLayoutDescriptor
    {
        // ... todo ...
    };


    /// <summary>
    /// Backend implemented pipeline layout operations.
    /// </summary>
    struct PipelineLayoutOperations
    {
        bool (*build)(PipelineLayoutDescriptor const&, PipelineLayoutHandle const&);
        void (*destroy)(PipelineLayoutHandle const&);
    };

    class PipelineLayout
    {
    public:

        PipelineLayout(PipelineLayoutOperations const* pOperations, PipelineLayoutHandle handle)
            : m_pBackendOperations(pOperations), m_backendHandle(handle)
        {

        }

        PipelineLayout(PipelineLayout const&) = delete;
        PipelineLayout& operator=(PipelineLayout const&) = delete;

        ~PipelineLayout()
        {
            destroy();
        }

        bool build(PipelineLayoutDescriptor const& descriptor)
        {
            m_descriptor = descriptor;
            return rebuild();
        }

        bool rebuild()
        {
            destroy();
            return m_pBackendOperations->build(m_descriptor, m_backendHandle);
        }

        void destroy()
        {
            if (m_backendHandle.handle != nullptr)
            {
                m_pBackendOperations->destroy(m_backendHandle);
                m_backendHandle.handle = nullptr;
            }
        }

        PipelineLayoutDescriptor const& getDescriptor() const noexcept
        {
            return m_descriptor;
        }

        PipelineLayoutHandle const* getHandle() const
        {
            return &m_backendHandle;
        }

    protected:

    private:

        PipelineLayoutDescriptor m_descriptor;
        PipelineLayoutHandle m_backendHandle;
        PipelineLayoutOperations const* m_pBackendOperations;
    };
}

#endif