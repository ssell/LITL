#ifndef LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__
#define LITL_VULKAN_RENDERER_COMMAND_BUFFER_H__

#include <vector>
#include <vulkan/vulkan.h>
#include "litl-renderer/commands/commandBuffer.hpp"

namespace LITL::Vulkan::Renderer
{
    class CommandBuffer : public LITL::Renderer::CommandBuffer
    {
    public:

        CommandBuffer(VkDevice vkDevice, VkCommandPool vkCommandPool, uint32_t framesInFlight);
        CommandBuffer(CommandBuffer const&) = delete;
        CommandBuffer const& operator=(CommandBuffer const&) = delete;

        ~CommandBuffer();

        bool build() override;
        bool begin(uint32_t frame) override;
        bool end(uint32_t frame) override;

        void cmdTransitionImageLayout(VkImage vkImage, uint32_t oldLayout, uint32_t newLayout, uint32_t srcAccessMask, uint32_t dstAccessMask, uint32_t srcStageMask, uint32_t dstStageMask);

    protected:

    private:

        VkCommandBuffer getCurrentCommandBuffer(uint32_t frame) const noexcept;

        VkDevice m_vkDevice;
        VkCommandPool m_vkCommandPool;
        std::vector<VkCommandBuffer> m_vkCommandBuffers;
    };
}

#endif