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
        inline uint64_t hash() const
        {
            return m_hash;
        }

        static PipelineLayoutDescriptor create()
        {
            PipelineLayoutDescriptor descriptor{};
            descriptor.m_hash = 0;
            return descriptor;
        }

    private:

        PipelineLayoutDescriptor() {}

        uint64_t m_hash;
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

        PipelineLayout(PipelineLayoutOperations const* pOperations, PipelineLayoutHandle handle, PipelineLayoutDescriptor const& descriptor)
            : m_pBackendOperations(pOperations), m_backendHandle(handle), m_descriptor(descriptor)
        {

        }

        PipelineLayout(PipelineLayout const&) = delete;
        PipelineLayout& operator=(PipelineLayout const&) = delete;

        ~PipelineLayout()
        {
            destroy();
        }

        bool build()
        {
            if (m_backendHandle.handle == nullptr)
            {
                return m_pBackendOperations->build(m_descriptor, m_backendHandle);
            }
            else
            {
                return false;
            }
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

        PipelineLayoutOperations const* m_pBackendOperations;
        PipelineLayoutHandle m_backendHandle;
        PipelineLayoutDescriptor m_descriptor;
    };
}

#endif