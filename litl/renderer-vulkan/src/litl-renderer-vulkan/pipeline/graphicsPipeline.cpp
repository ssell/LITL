#include "litl-renderer-vulkan/pipeline/graphicsPipeline.hpp"

namespace LITL::Vulkan::Renderer
{
    GraphicsPipeline::GraphicsPipeline(VkDevice vkDevice, LITL::Renderer::GraphicsPipelineDescriptor const& descriptor)
        : m_vkDevice(vkDevice)
    {
        m_descriptor = descriptor;
    }
}