#ifndef LITL_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_RENDERER_PIPELINE_LAYOUT_H__

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
    /// Optional additional data needed by the backend to create object.
    /// </summary>
    struct PipelineLayoutData
    {
        LITLHandle handle;
    };

    /// <summary>
    /// Opaque handle to the backend pipeline layout object.
    /// </summary>
    struct PipelineLayoutHandle
    {
        LITLHandle handle;
    };

    /// <summary>
    /// Backend provided pipeline layout operations.
    /// </summary>
    struct PipelineLayoutOperations
    {
        bool (*build)(PipelineLayoutData const&, PipelineLayoutDescriptor const&, PipelineLayoutHandle&);
        void (*destroy)(PipelineLayoutHandle const&);
    };

    class PipelineLayout
    {
    public:

        PipelineLayout() = default;
        PipelineLayout(PipelineLayout const&) = delete;
        PipelineLayout& operator=(PipelineLayout const&) = delete;

        ~PipelineLayout()
        {
            destroy();
        }

        bool build(PipelineLayoutData const& data, PipelineLayoutDescriptor const& descriptor)
        {
            m_backendData = data;
            m_descriptor = descriptor;

            return rebuild();
        }

        bool rebuild()
        {
            destroy();
            return m_backendOperations->build(m_backendData, m_descriptor, m_backendHandle);
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
        PipelineLayoutData m_backendData;
        PipelineLayoutHandle m_backendHandle;
        PipelineLayoutOperations const* m_backendOperations;
    };
}

#endif