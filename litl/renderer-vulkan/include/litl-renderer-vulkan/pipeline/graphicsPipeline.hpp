#ifndef LITL_VULKAN_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_VULKAN_RENDERER_GRAPHICS_PIPELINE_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/graphicsPipeline.hpp"

namespace LITL::Vulkan::Renderer
{
    class GraphicsPipeline : public LITL::Renderer::GraphicsPipeline
    {
    public:

        GraphicsPipeline(VkDevice vkDevice, LITL::Renderer::GraphicsPipelineDescriptor const& descriptor);
        GraphicsPipeline(GraphicsPipeline const&) = delete;
        GraphicsPipeline& operator=(GraphicsPipeline const&) = delete;

        ~GraphicsPipeline();

        void bind(LITL::Renderer::CommandBuffer* pCommandBuffer) noexcept override;

        VkPipeline getVkPipeline() const noexcept;

    protected:

    private:

        VkDevice m_vkDevice;
        VkPipeline m_vkPipeline;
    };
}

#endif