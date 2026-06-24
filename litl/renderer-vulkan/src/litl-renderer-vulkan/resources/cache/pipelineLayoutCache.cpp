#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/conversions.hpp"
#include "litl-renderer-vulkan/resources/cache/pipelineLayoutCache.hpp"

namespace litl::vulkan
{
    void PipelineLayoutCache::build(VkDevice vkDevice) noexcept
    {
        LITL_FATAL_ASSERT_MSG(m_vkDevice == VK_NULL_HANDLE, "Attempting to call PipelineLayoutCache::build twice");
        m_vkDevice = vkDevice;
    }

    void PipelineLayoutCache::destroy() noexcept
    {
        if (m_vkDevice != VK_NULL_HANDLE)
        {
            // Pipeline Layouts must be destroyed BEFORE the DescriptorSetLayouts, as they may reference them.
            for (auto& [_, pipelineLayout] : m_pipelineLayoutMap)
            {
                vkDestroyPipelineLayout(m_vkDevice, pipelineLayout, nullptr);
            }

            for (auto& [_, descriptorSetLayout] : m_descriptorSetLayoutMap)
            {
                vkDestroyDescriptorSetLayout(m_vkDevice, descriptorSetLayout, nullptr);
            }

            m_pipelineLayoutMap.clear();
            m_descriptorSetLayoutMap.clear();
            m_vkDevice = VK_NULL_HANDLE;
        }
    }

    // -------------------------------------------------------------------------------------
    // Get or Create VkDescriptorSetLayout
    // -------------------------------------------------------------------------------------

    VkDescriptorSetLayout createVkDescriptorSetLayout(VkDevice vkDevice, DescriptorSetLayoutDesc const& descriptorSetLayoutDesc, bool isPushSet) noexcept
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(descriptorSetLayoutDesc.bindings.size());

        for (auto const& binding : descriptorSetLayoutDesc.bindings)
        {
            bindings.push_back(VkDescriptorSetLayoutBinding{
                .binding = binding.binding,
                .descriptorType = toVkDescriptorType(binding.type),
                .descriptorCount = binding.arraySize,
                .stageFlags = toVkShaderStageFlags(binding.stages),
                .pImmutableSamplers = nullptr
                });
        }

        VkDescriptorSetLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = 0u,
            .bindingCount = static_cast<uint32_t>(descriptorSetLayoutDesc.bindings.size()),
            .pBindings = bindings.data()
        };

        if (isPushSet)
        {
            info.flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
        }

        VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;
        const VkResult result = vkCreateDescriptorSetLayout(vkDevice, &info, nullptr, &vkDescriptorSetLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkDescriptorSetLayout with result ", result);
            return VK_NULL_HANDLE;
        }

        return vkDescriptorSetLayout;
    }

    VkDescriptorSetLayout PipelineLayoutCache::getOrCreateSetLayout(DescriptorSetLayoutDesc const& descriptorSetLayoutDesc, DescriptorSetIndex index) noexcept
    {
        LITL_ASSERT_MSG(m_vkDevice != VK_NULL_HANDLE, "Attempting to use Vulkan PipelineLayoutCache without providing a VkDevice", VK_NULL_HANDLE);

        // Get
        auto find = m_descriptorSetLayoutMap.find(descriptorSetLayoutDesc);

        if (find != m_descriptorSetLayoutMap.end())
        {
            return find->second;
        }

        // Create
        auto vkDescriptorSetLayout = createVkDescriptorSetLayout(m_vkDevice, descriptorSetLayoutDesc, (index == DescriptorSetIndex::PerObject));

        if (vkDescriptorSetLayout != VK_NULL_HANDLE)
        {
            m_descriptorSetLayoutMap[descriptorSetLayoutDesc] = vkDescriptorSetLayout;
        }

        return vkDescriptorSetLayout;
    }

    // -------------------------------------------------------------------------------------
    // Get or Create VkPipelineLayout
    // -------------------------------------------------------------------------------------

    VkPipelineLayout createVkPipelineLayout(VkDevice vkDevice, PipelineLayoutCacheKey const& pipelineLayout) noexcept
    {
        std::vector<VkPushConstantRange> pushConstants;
        pushConstants.reserve(pipelineLayout.pushConstants.size());

        for (auto const& pushConstant : pipelineLayout.pushConstants)
        {
            pushConstants.push_back(VkPushConstantRange{
                .stageFlags = toVkShaderStageFlags(pushConstant.stages),
                .offset = pushConstant.offset,
                .size = pushConstant.sizeBytes
                });
        }

        VkPipelineLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(pipelineLayout.setLayoutHandles.size()),
            .pSetLayouts = pipelineLayout.setLayoutHandles.data(),
            .pushConstantRangeCount = static_cast<uint32_t>(pipelineLayout.pushConstants.size()),
            .pPushConstantRanges = pushConstants.data()
        };

        VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
        const VkResult result = vkCreatePipelineLayout(vkDevice, &info, nullptr, &vkPipelineLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkPipelineLayout with result ", result);
            return VK_NULL_HANDLE;
        }

        return vkPipelineLayout;
    }

    VkPipelineLayout PipelineLayoutCache::getOrCreatePipelineLayout(PipelineLayoutDescriptor const& pipelineLayoutDesc) noexcept
    {
        LITL_ASSERT_MSG(m_vkDevice != VK_NULL_HANDLE, "Attempting to use Vulkan PipelineLayoutCache without providing a VkDevice", VK_NULL_HANDLE);

        // Build up the cache key that we will be looking for.
        PipelineLayoutCacheKey cacheKey;
        cacheKey.pushConstants = pipelineLayoutDesc.pushConstants;
        cacheKey.setLayoutHandles.reserve(pipelineLayoutDesc.setLayouts.size());

        LITL_ASSERT_MSG((pipelineLayoutDesc.setLayouts.size() <= static_cast<size_t>(DescriptorSetIndex::DescriptorSetMaxCount)), "Pipeline Layout Descriptor Set count exceeds expected maximum count.", VK_NULL_HANDLE);

        for (auto i = 0ull; i < pipelineLayoutDesc.setLayouts.size(); ++i)
        {
            auto& setLayout = pipelineLayoutDesc.setLayouts[i];
            cacheKey.setLayoutHandles.push_back(getOrCreateSetLayout(setLayout, static_cast<DescriptorSetIndex>(i)));
        }

        // Get
        auto find = m_pipelineLayoutMap.find(cacheKey);

        if (find != m_pipelineLayoutMap.end())
        {
            return find->second;
        }

        // Create
        auto vkPipelineLayout = createVkPipelineLayout(m_vkDevice, cacheKey);

        if (vkPipelineLayout != VK_NULL_HANDLE)
        {
            m_pipelineLayoutMap.emplace(std::move(cacheKey), vkPipelineLayout);
        }

        return vkPipelineLayout;
    }
}