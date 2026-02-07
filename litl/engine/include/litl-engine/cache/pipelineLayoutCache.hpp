#ifndef LITL_ENGINE_PIPELINE_LAYOUT_CACHE_H__
#define LITL_ENGINE_PIPELINE_LAYOUT_CACHE_H__

#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Engine
{
    class PipelineLayoutCache
    {
    public:

        PipelineLayoutCache();
        ~PipelineLayoutCache();

        PipelineLayoutCache(PipelineLayoutCache const&) = delete;
        PipelineLayoutCache& operator=(PipelineLayoutCache const&) = delete;

        Renderer::PipelineLayout* getPipelineLayout(Renderer::PipelineLayoutDescriptor const& descriptor);

    protected:

    private:


    };
}

#endif