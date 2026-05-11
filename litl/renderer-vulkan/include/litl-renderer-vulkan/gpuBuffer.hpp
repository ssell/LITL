#ifndef LITL_VULKAN_RENDERER_GPU_BUFFER_H__
#define LITL_VULKAN_RENDERER_GPU_BUFFER_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/gpuBuffer.hpp"

namespace litl::vulkan
{
    /// <summary>
    /// Handle to a VkBuffer using Buffer Device Address (BDA).
    /// </summary>
    struct GpuBufferHandle
    {
        /// <summary>
        /// The logical device used to create the buffer.
        /// </summary>
        VkDevice vkDevice;

        /// <summary>
        /// The physical device used to create the buffer.
        /// </summary>
        VkPhysicalDevice vkPhysicalDevice;

        /// <summary>
        /// The size of the buffer.
        /// </summary>
        VkDeviceSize vkDeviceSize;

        /// <summary>
        /// The buffer.
        /// </summary>
        VkBuffer vkBuffer;

        /// <summary>
        /// The buffer allocated memory.
        /// </summary>
        VkDeviceMemory vkDeviceMemory;

        /// <summary>
        /// The stable address of the memory buffer.
        /// </summary>
        VkDeviceAddress vkDeviceAddress;
    };

    litl::GpuBuffer* createVulkanGpuBuffer(VkDevice vkDevice, VkPhysicalDevice vkPhysicalDevice, VkDeviceSize vkDeviceSize);

    bool build(litl::GpuBufferHandle const& litlHandle) noexcept;
    void destroy(litl::GpuBufferHandle const& litlHandle) noexcept;
    void write(litl::GpuBufferHandle const& litlHandle, void* data, uint32_t offset, uint32_t size) noexcept;
    uint32_t size(litl::GpuBufferHandle const& litlHandle) noexcept;

    const litl::GpuBufferOperations VulkanGpuBufferOperations = {
        &build,
        &destroy,
        &write,
        &size
    };
}

#endif