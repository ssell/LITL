#ifndef LITL_RENDERER_RESOURCE_MANAGER_H__
#define LITL_RENDERER_RESOURCE_MANAGER_H__

#include "litl-renderer/handles.hpp"
#include "litl-renderer/pipeline/graphicsPipeline.hpp"

namespace litl
{
    class GraphicsPipelineV2 final
    {
    public:

        GraphicsPipelineV2(GraphicsPipelineHandleV2 handle, GraphicsPipelineDescriptor const& descriptor)
            : m_handle(handle), m_descriptor(descriptor)
        {

        }

    private:

        GraphicsPipelineHandleV2 m_handle;
        GraphicsPipelineDescriptor m_descriptor;
    };


    struct ResourceManagerOperations
    {
        bool (*build)(ResourceManagerHandle const&);
        void (*destroy)(ResourceManagerHandle const&);

    };

    class ResourceManager final
    {
    public:

    private:
    };
}

#endif