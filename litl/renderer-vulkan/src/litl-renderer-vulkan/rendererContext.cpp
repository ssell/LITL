#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    uint32_t DeviceInfo::getUabRuntimeArrayCapacityFor(ShaderResourceType type) const noexcept
    {
        switch (type)
        {
        case ShaderResourceType::Sampler:
            return min(vkIndexingProperties.maxPerStageDescriptorUpdateAfterBindSamplers, vkIndexingProperties.maxDescriptorSetUpdateAfterBindSamplers);

        case ShaderResourceType::UniformBuffer:
            return min(vkIndexingProperties.maxPerStageDescriptorUpdateAfterBindUniformBuffers, vkIndexingProperties.maxDescriptorSetUpdateAfterBindUniformBuffers);

        case ShaderResourceType::StorageBuffer:
            return min(vkIndexingProperties.maxPerStageDescriptorUpdateAfterBindStorageBuffers, vkIndexingProperties.maxDescriptorSetUpdateAfterBindStorageBuffers);

        case ShaderResourceType::SampledImage:
            return min(vkIndexingProperties.maxPerStageDescriptorUpdateAfterBindSampledImages, vkIndexingProperties.maxDescriptorSetUpdateAfterBindSampledImages);

        case ShaderResourceType::StorageImage:
        case ShaderResourceType::ImageBuffer:
            return min(vkIndexingProperties.maxPerStageDescriptorUpdateAfterBindStorageImages, vkIndexingProperties.maxDescriptorSetUpdateAfterBindStorageImages);

        case ShaderResourceType::InputAttachment:
            return min(vkIndexingProperties.maxPerStageDescriptorUpdateAfterBindInputAttachments, vkIndexingProperties.maxDescriptorSetUpdateAfterBindInputAttachments);

        case ShaderResourceType::AccelerationStructure:
            // Limits live in VkPhysicalDeviceAccelerationStructurePropertiesKHR, which requires VK_KHR_acceleration_structure. 
            // Not currently enabled, so a runtime array of these is unreachable.
            return 0u;

        case ShaderResourceType::Unknown:
        default:
            return 0u;
        }
    }

    PerFrameSyncInfo& RendererContext::getCurrFrameSyncInfo() noexcept
    {
        return renderInfo.frameSyncInfo[renderInfo.frame.frameInFlightIndex];
    }

    PerFrameSyncInfo& RendererContext::getPrevFrameSyncInfo() noexcept
    {
        const uint32_t prevFrameInFlightIndex = (renderInfo.frame.frameCount + (renderInfo.frame.framesInFlight - 1)) % renderInfo.frame.framesInFlight;
        return renderInfo.frameSyncInfo[prevFrameInFlightIndex];
    }

    PerImageSyncInfo& RendererContext::getCurrImageSyncInfo() noexcept
    {
        return renderInfo.imageSyncInfo[swapChain.swapChainImageIndex];
    }

    TextureResourceHandle RendererContext::getCurrFrameDepthTexture() noexcept
    {
        return getCurrFrameSyncInfo().depthTexture;
    }

    TextureResourceHandle RendererContext::getPrevFrameDepthTexture() noexcept
    {
        return getPrevFrameSyncInfo().depthTexture;
    }
}