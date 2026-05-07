#include "litl-core/handles.hpp"
#include "litl-renderer-vulkan/gpuBuffer.hpp"

namespace litl::vulkan
{
    litl::GpuBuffer* createGpuBuffer(VkDevice vkDevice, VkDeviceMemory vkDeviceMemory)
    {
        auto* gpuBufferHandle = new GpuBufferHandle{
            .vkDevice = vkDevice,
            .vkDeviceMemory = vkDeviceMemory,
            .vkBuffer = VK_NULL_HANDLE
        };

        return new litl::GpuBuffer(
            &VulkanGpuBufferOperations,
            LITL_PACK_HANDLE(litl::GpuBufferHandle, gpuBufferHandle));
    }

    bool build(litl::GpuBufferHandle const& litlHandle) noexcept
    {
        return false;
    }

    void destroy(litl::GpuBufferHandle const& litlHandle) noexcept
    {

    }
    
    void* map(litl::GpuBufferHandle const& litlHandle) noexcept
    {
        return nullptr;
    }

    void unmap(litl::GpuBufferHandle const& litlHandle) noexcept
    {

    }

    uint32_t size(litl::GpuBufferHandle const& litlHandle) noexcept
    {
        return 0;
    }
}