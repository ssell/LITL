#ifndef LITL_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_RENDERER_PIPELINE_LAYOUT_H__

#include <memory>
#include "litl-renderer/handles.hpp"

namespace LITL::Renderer
{
    /// <summary>
    /// Configuration for the pipeline layout.
    /// </summary>
    struct PipelineLayoutDescriptor
    {
        // ... todo ...
    };

    /// <summary>
    /// Opaque handle to the backend pipeline layout object.
    /// </summary>
    struct PipelineLayoutHandle
    {
        LITLHandle handle;
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

        PipelineLayout(PipelineLayoutOperations const* operations, PipelineLayoutHandle handle)
        {
            m_backendOperations = operations;
            m_backendHandle = handle;
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
            return m_backendOperations->build(m_descriptor, m_backendHandle);
        }

        void destroy()
        {
            if (m_backendHandle.handle != nullptr)
            {
                m_backendOperations->destroy(m_backendHandle);
                m_backendHandle.handle = nullptr;
            }
        }

        PipelineLayoutDescriptor const& getDescriptor() const noexcept
        {
            return m_descriptor;
        }

    protected:

    private:

        PipelineLayoutDescriptor m_descriptor;
        PipelineLayoutHandle m_backendHandle;
        PipelineLayoutOperations const* m_backendOperations;
    };
}

#endif