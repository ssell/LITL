#include "litl-core/logging/logging.hpp"
#include "litl-core/handles.hpp"
#include "litl-renderer-vulkan/gpuBuffer.hpp"

namespace litl::vulkan
{
    litl::GpuBuffer* createGpuBuffer(VkDevice vkDevice, VkPhysicalDevice vkPhysicalDevice, VkDeviceSize vkDeviceSize)
    {
        auto* gpuBufferHandle = new GpuBufferHandle{
            .vkDevice = vkDevice,
            .vkPhysicalDevice = vkPhysicalDevice,
            .vkDeviceSize = vkDeviceSize,
            .vkBuffer = VK_NULL_HANDLE,
            .vkDeviceMemory = VK_NULL_HANDLE,
            .vkDeviceAddress = 0                // "A value of zero is reserved as a “null” pointer and must not be returned as a valid buffer device address."
        };

        return new litl::GpuBuffer(
            &VulkanGpuBufferOperations,
            LITL_PACK_HANDLE(litl::GpuBufferHandle, gpuBufferHandle));
    }

    bool build(litl::GpuBufferHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(GpuBufferHandle, litlHandle);

        if (handle->vkDeviceMemory != VK_NULL_HANDLE || handle->vkBuffer != VK_NULL_HANDLE)
        {
            // buffer already exists
            return false;
        }

        // Create the buffer
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = handle->vkDeviceSize;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferCreateInfo.usage =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |                // can be used in a descriptor set or dynamic storage
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |         // can be used in shaders
            VK_BUFFER_USAGE_TRANSFER_DST_BIT;                   // can be used as the destination of a transfer command

        VkResult result = vkCreateBuffer(handle->vkDevice, &bufferCreateInfo, nullptr, &handle->vkBuffer);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan GPU Buffer with result ", result);
            return false;
        }

        // Find a suitable memory type
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(handle->vkDevice, handle->vkBuffer, &memoryRequirements);

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(handle->vkPhysicalDevice, &memoryProperties);

        VkMemoryPropertyFlags requiredProperties =              // together creates memory we can map and write directly. todo: use staging buffers instead
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |               // can be mapped for host access using vkMapMemory
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;               // specifies that the host cache management commands (flush/invalidate) are not needed

        uint32_t memoryTypeIndex = UINT32_MAX;

        for (uint32_t i = 0u; i < memoryProperties.memoryTypeCount; ++i)
        {
            if ((memoryRequirements.memoryTypeBits & (1u << i)) &&
                (memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties)
            {
                memoryTypeIndex = i;
                break;
            }
        }

        // Specify it will use Buffer Device Address (BDA)
        VkMemoryAllocateFlagsInfo flagsInfo{};
        flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = &flagsInfo;
        allocateInfo.allocationSize = memoryRequirements.size;
        allocateInfo.memoryTypeIndex = memoryTypeIndex;

        result = vkAllocateMemory(handle->vkDevice, &allocateInfo, nullptr, &handle->vkDeviceMemory);

        if (result != VK_SUCCESS)
        {
            logError("Failed to allocate memory for Vulkan GPU Buffer with result ", result);
            return false;
        }

        result = vkBindBufferMemory(handle->vkDevice, handle->vkBuffer, handle->vkDeviceMemory, 0);

        if (result != VK_SUCCESS)
        {
            logError("Faield to bind Vulkan GPU Buffer to its allocated memory with result ", result);
            return false;
        }

        VkBufferDeviceAddressInfo deviceAddressInfo{};
        deviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAddressInfo.buffer = handle->vkBuffer;

        handle->vkDeviceAddress = vkGetBufferDeviceAddress(handle->vkDevice, &deviceAddressInfo);

        // "A value of zero is reserved as a “null” pointer and must not be returned as a valid buffer device address."
        if (handle->vkDeviceAddress == 0)
        {
            logError("Failed to retrieve Vulkan GPU Buffer Device Address");
            return false;
        }

        return true;
    }

    void destroy(litl::GpuBufferHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(GpuBufferHandle, litlHandle);

        if (handle->vkDeviceMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(handle->vkDevice, handle->vkDeviceMemory, nullptr);
            handle->vkDeviceMemory = VK_NULL_HANDLE;
        }

        if (handle->vkBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(handle->vkDevice, handle->vkBuffer, nullptr);
            handle->vkBuffer = nullptr;
        }

        delete handle;
    }

    void write(litl::GpuBufferHandle const& litlHandle, void* data, uint32_t offset, uint32_t size) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(GpuBufferHandle, litlHandle);
        void* mapped = nullptr;

        VkResult result = vkMapMemory(handle->vkDevice, handle->vkDeviceMemory, offset, size, 0 /* flags */, &mapped);

        if (result != VK_SUCCESS)
        {
            logError("Failed to map memory to Vulkan GPU Buffer with result ", result);
            return;
        }

        std::memcpy(mapped, data, size);

        vkUnmapMemory(handle->vkDevice, handle->vkDeviceMemory);
    }

    uint32_t size(litl::GpuBufferHandle const& litlHandle) noexcept
    {
        auto* handle = LITL_UNPACK_HANDLE(GpuBufferHandle, litlHandle);
        return handle->vkDeviceSize;
    }
}