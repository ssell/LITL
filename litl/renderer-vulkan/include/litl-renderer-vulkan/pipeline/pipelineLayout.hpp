#ifndef LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    class PipelineLayout : public LITL::Renderer::PipelineLayout
    {
    public:

        PipelineLayout(VkDevice vkDevice, LITL::Renderer::PipelineLayoutDescriptor const& descriptor);
        ~PipelineLayout();

        bool build() noexcept;
        bool rebuild(LITL::Renderer::PipelineLayoutDescriptor const& descriptor) noexcept override;

        VkPipelineLayout getVkPipelineLayout() const noexcept;

    protected:

    private:

        void cleanup();

        VkDevice m_vkDevice;
        VkPipelineLayout m_vkPipelineLayout;
    };
}

#endif