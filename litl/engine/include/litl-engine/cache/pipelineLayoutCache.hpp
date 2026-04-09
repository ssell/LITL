#ifndef LITL_ENGINE_PIPELINE_LAYOUT_CACHE_H__
#define LITL_ENGINE_PIPELINE_LAYOUT_CACHE_H__

#include "litl-core/impl.hpp"
#include "litl-renderer/pipeline/pipelineLayout.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    class PipelineLayoutCache
    {
    public:

        PipelineLayoutCache(Renderer const* pRenderer);
        ~PipelineLayoutCache();

        PipelineLayoutCache(PipelineLayoutCache const&) = delete;
        PipelineLayoutCache& operator=(PipelineLayoutCache const&) = delete;

        PipelineLayout* getPipelineLayout(PipelineLayoutDescriptor const& descriptor);

    protected:

    private:

        struct Impl;
        ImplPtr<Impl, 96> m_impl;
    };
}

#endif