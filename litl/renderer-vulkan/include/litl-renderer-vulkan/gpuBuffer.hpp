#ifndef LITL_VULKAN_RENDERER_GPU_BUFFER_H__
#define LITL_VULKAN_RENDERER_GPU_BUFFER_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/gpuBuffer.hpp"

namespace litl::vulkan
{
    struct GpuBufferHandle
    {
        VkDevice vkDevice;
        VkDeviceMemory vkDeviceMemory;
        VkBuffer vkBuffer;
    };

    litl::GpuBuffer* createVulkanGpuBuffer(VkDevice vkDevice, VkDeviceMemory vkDeviceMemory);

    bool build(litl::GpuBufferHandle const& litlHandle) noexcept;
    void destroy(litl::GpuBufferHandle const& litlHandle) noexcept;
    void* map(litl::GpuBufferHandle const& litlHandle) noexcept;
    void unmap(litl::GpuBufferHandle const& litlHandle) noexcept;
    uint32_t size(litl::GpuBufferHandle const& litlHandle) noexcept;

    const litl::GpuBufferOperations VulkanGpuBufferOperations = {
        &build,
        &destroy,
        &map,
        &unmap,
        &size
    };
}

#endif