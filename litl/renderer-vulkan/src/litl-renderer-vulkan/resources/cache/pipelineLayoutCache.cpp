#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/conversions.hpp"
#include "litl-renderer-vulkan/resources/cache/pipelineLayoutCache.hpp"

namespace litl::vulkan
{
    void PipelineLayoutCache::build(VkDevice vkDevice, uint32_t textureTableCapacity) noexcept
    {
        LITL_FATAL_ASSERT_MSG(m_vkDevice == VK_NULL_HANDLE, "Attempting to call PipelineLayoutCache::build twice");
        m_vkDevice = vkDevice;
        m_textureTableCapacity = textureTableCapacity;
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

    VkDescriptorSetLayout createVkDescriptorSetLayout(VkDevice vkDevice, DescriptorSetLayoutDesc const& descriptorSetLayoutDesc, DescriptorSetLayoutOptions const& options) noexcept
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkDescriptorBindingFlags> bindingFlags;                 // Parallel, 0 for ordinary bindings.

        bindings.reserve(descriptorSetLayoutDesc.bindings.size());
        bindingFlags.reserve(descriptorSetLayoutDesc.bindings.size());
        
        bool hasRuntimeArray = false;

        for (auto const& binding : descriptorSetLayoutDesc.bindings)
        {
            const bool isRuntimeArray = (binding.arraySize == 0u);
            hasRuntimeArray |= isRuntimeArray;

            bindings.push_back(VkDescriptorSetLayoutBinding{
                .binding = binding.binding,
                .descriptorType = toVkDescriptorType(binding.type),
                .descriptorCount = (isRuntimeArray ? options.runtimeArrayCapacity : binding.arraySize),
                .stageFlags = toVkShaderStageFlags(binding.stages),
                .pImmutableSamplers = nullptr
            });

            if (isRuntimeArray)
            {
                bindingFlags.push_back(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
            }
            else
            {
                // flags binding count must be 0 or match bindings count exactly. We push zero for ordinary bindings to ensure that it always aligns.
                bindingFlags.push_back(VkDescriptorBindingFlags{ 0 });
            }
        }

        LITL_ASSERT_MSG(!(options.isPushDescriptor && hasRuntimeArray), "Descriptor set layout cannot be both push-descriptor and update-after-bind", VK_NULL_HANDLE);      // A push-descriptor set layout may not contain UPDATE_AFTER_BIND bindings.
        LITL_ASSERT_MSG(!hasRuntimeArray || (options.runtimeArrayCapacity > 0u), "Runtime descriptor array requested with a capacity of zero", VK_NULL_HANDLE);             // A runtime array with capacity 0 would silently become a reserved/skipped binding.

        const VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data()
        };

        VkDescriptorSetLayoutCreateFlags layoutFlags = 0;

        if (options.isPushDescriptor)
        {
            layoutFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT;
        }

        if (hasRuntimeArray)
        {
            layoutFlags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }

        const VkDescriptorSetLayoutCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &flagsInfo,
            .flags = layoutFlags,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };

        VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;
        const VkResult result = vkCreateDescriptorSetLayout(vkDevice, &createInfo, nullptr, &vkDescriptorSetLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkDescriptorSetLayout with result ", result);
            return VK_NULL_HANDLE;
        }

        return vkDescriptorSetLayout;
    }

    VkDescriptorSetLayout PipelineLayoutCache::getOrCreateSetLayout(DescriptorSetLayoutDesc const& descriptorSetLayoutDesc, uint32_t setIndex) noexcept
    {
        LITL_ASSERT_MSG(m_vkDevice != VK_NULL_HANDLE, "Attempting to use Vulkan PipelineLayoutCache without providing a VkDevice", VK_NULL_HANDLE);

        // Resolve the cache key once and then use for both the lookup and creation
        const DescriptorSetLayoutCacheKey cacheKey{
            .desc = descriptorSetLayoutDesc,
            .options = DescriptorSetLayoutOptions{
                .runtimeArrayCapacity = m_textureTableCapacity,
                .isPushDescriptor = (static_cast<DescriptorSetIndex>(setIndex) == DescriptorSetIndex::PerObject)
            }
        };

        const auto find = m_descriptorSetLayoutMap.find(cacheKey);

        if (find != m_descriptorSetLayoutMap.end())
        {
            return find->second;
        }

        const auto vkDescriptorSetLayout = createVkDescriptorSetLayout(m_vkDevice, cacheKey.desc, cacheKey.options);

        if (vkDescriptorSetLayout != VK_NULL_HANDLE)
        {
            m_descriptorSetLayoutMap[cacheKey] = vkDescriptorSetLayout;
        }

        return vkDescriptorSetLayout;
    }

    // -------------------------------------------------------------------------------------
    // Get or Create VkPipelineLayout
    // -------------------------------------------------------------------------------------

    VkPipelineLayout createVkPipelineLayout(VkDevice vkDevice, PipelineLayoutCacheKey const& pipelineLayout) noexcept
    {
        // While we have the actual exact push constant ranges via reflection, we instead assign a general global range of all shaders and 128 bytes.
        // This is to avoid potential silent bugs where a pipeline is bound with an incompatible push constant layout than what is currently bound and
        // the user forgets to bind the new push constant layout. So all we have to do instead is enforce that no push constant structure exceedes this size.

        const VkPushConstantRange pushConstantRange {
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0u,
            .size = 128u
        };

        VkPipelineLayoutCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>(pipelineLayout.setLayoutHandles.size()),
            .pSetLayouts = pipelineLayout.setLayoutHandles.data(),
            .pushConstantRangeCount = 1u,
            .pPushConstantRanges = &pushConstantRange
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

        for (uint32_t i = 0u; i < pipelineLayoutDesc.setLayouts.size(); ++i)
        {
            auto& setLayout = pipelineLayoutDesc.setLayouts[i];
            cacheKey.setLayoutHandles.push_back(getOrCreateSetLayout(setLayout, i));
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