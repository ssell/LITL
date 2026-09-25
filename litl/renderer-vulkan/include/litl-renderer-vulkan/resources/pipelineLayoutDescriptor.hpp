#ifndef LITL_RENDERER_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_H__
#define LITL_RENDERER_VULKAN_PIPELINE_LAYOUT_DESCRIPTOR_H__

#include <cstdint>
#include <optional>

#include "litl-core/hash.hpp"
#include "litl-renderer/reflection.hpp"
#include "litl-renderer/resources/shaderModule.hpp"

namespace litl::vulkan
{
    struct ShaderModuleResource;

    /// <summary>
    /// A singular resource descriptor binding. Each individual descriptor binding is
    /// specified by a descriptor/resource type, an array size, and the set of shader
    /// stages that can access the binding.
    /// 
    /// This structure mimics the Vulkan design of having the set be implicit based
    /// on it position (index) in the owning DescriptorSetLayoutDesc.
    /// </summary>
    struct DescriptorSetLayoutBindingDesc
    {
        /// <summary>
        /// Hash of the string name.
        /// </summary>
        StringId id;

        /// <summary>
        /// Vulkan binding, D3D12 register.
        /// </summary>
        uint32_t binding;

        /// <summary>
        /// Buffer, image, sampler, etc.
        /// </summary>
        ShaderResourceType type;

        /// <summary>
        /// 0 = runtime bindless array, 1 = not array, >=2 = array of declared size
        /// </summary>
        uint32_t arraySize;

        /// <summary>
        /// Bitmask of one or more ShaderStage that reference this binding.
        /// </summary>
        ShaderStage stages;

        bool operator==(DescriptorSetLayoutBindingDesc const&) const = default;
    };

    static_assert(sizeof(DescriptorSetLayoutBindingDesc) == 6 * sizeof(uint32_t), "Layout has padding; bytewise hash is unsafe");

    /// <summary>
    /// An array of zero or more descriptor set resource bindings.
    /// </summary>
    struct DescriptorSetLayoutDesc
    {
        /// <summary>
        /// Sorted by DescriptorSetLayoutBindingDesc::binding (ascending)
        /// </summary>
        std::vector<DescriptorSetLayoutBindingDesc> bindings;

        bool operator==(DescriptorSetLayoutDesc const& other) const
        {
            return bindings == other.bindings;
        }
    };

    /// <summary>
    /// The runtime array maximum capacities for each shader resource type.
    /// </summary>
    struct DescriptorSetRuntimeArrayCapacities
    {
        uint32_t accelerationStructure{ 0u };
        uint32_t imageBuffer{ 0u };
        uint32_t inputAttachment{ 0u };
        uint32_t sampledImage{ 0u };
        uint32_t sampler{ 0u };
        uint32_t storageBuffer{ 0u };
        uint32_t storageImage{ 0u };
        uint32_t uniformBuffer{ 0u };

        [[nodiscard]] bool operator==(DescriptorSetRuntimeArrayCapacities const&) const = default;
        [[nodiscard]] uint32_t getFor(ShaderResourceType resourceType) const noexcept;
        [[nodiscard]] bool hasZeroCapacity() const noexcept;
    };

    struct DescriptorSetLayoutOptions
    {
        /// <summary>
        /// The array capacities for each shader resource type that is used by the descriptor set.
        /// </summary>
        DescriptorSetRuntimeArrayCapacities capacities{ 0u };

        /// <summary>
        /// Descriptor Set index 3 only. Mutally exclusive with runtime arrays.
        /// 
        /// Note that we key on "pushness" rather than individual set indices because sets 0, 1, 2 all result in the same layout.
        /// Whether or not it is a push descriptor is what really changes the creation call.
        /// </summary>
        bool isPushDescriptor{ false };

        [[nodiscard]] bool operator==(DescriptorSetLayoutOptions const&) const = default;
    };

    /// <summary>
    /// Identity of a creted VkDescriptorSetLayout: what the shader asked for (desc) plus the creation policy applied to it (options).
    /// Both participate because both change the resulting object.
    /// </summary>
    struct DescriptorSetLayoutCacheKey
    {
        DescriptorSetLayoutDesc desc{};
        DescriptorSetLayoutOptions options{};

        [[nodiscard]] bool operator==(DescriptorSetLayoutCacheKey const& other) const noexcept
        {
            return (options == other.options) && (desc == other.desc);
        }
    };

    /// <summary>
    /// Description of a single Pipeline Layout.
    /// A Pipeline Layout describes the layout/structure of data bound to the GPU in two forms:
    /// Descriptor Set Layouts (composed of zero or more Descriptor Set Bindings) and Push Constants.
    /// 
    /// These are typically built via Shader Reflection and generally should not be made by hand.
    /// </summary>
    struct PipelineLayoutDescriptor
    {
        /// <summary>
        /// All descriptor set layouts present in the pipeline layout.
        /// A pipeline layout has multiple descriptor set layouts that are distinguished by their binding frequency. For example:
        /// 
        ///     setLayouts[DescriptorSetIndex::PerFrame] =
        ///       bindings[0] = view matrix
        ///       bindings[1] = proj matrix
        ///       bindings[2] = time/frame info
        ///       bindings[3] = camera position
        /// 
        ///     setLayouts[DescriptorSetIndex::PerPass] =
        ///       bindings[0] = shadow map
        /// 
        ///     setLayouts[DescriptorSetIndex::PerMaterial] =
        ///       bindings[0] = albedo map
        ///       bindings[1] = normal map
        /// 
        ///     setLayouts[DescriptorSetIndex::PerObject] =
        ///       bindings[0] = object index into SSBOs
        /// </summary>
        std::vector<DescriptorSetLayoutDesc> setLayouts;

        /// <summary>
        /// All push constants defined for the pipeline layout.
        /// Each range occupies a non-overlapping region of the shared push constant block (max 128 bytes for portability). 
        /// Stages sharing a range have their visibility flags OR'd together; partial overlaps across stages are rejected at merge time.
        /// </summary>
        std::vector<MergedPushConstantRange> pushConstants;
    };

    /// <summary>
    /// Resource and entry point for a specific reflected shader used in the construction of a PipelineLayoutDescriptor.
    /// </summary>
    struct PipelineLayoutDescriptorShaderModuleInfo
    {
        ShaderModuleResource const* resource = nullptr;
        ShaderStage stage;
        std::string entryPoint;
    };

    /// <summary>
    /// Info used to merge reflected shader data into a single PipelineLayoutDescriptor.
    /// </summary>
    struct PipelineLayoutDescriptorCreateInfo
    {
        std::vector<PipelineLayoutDescriptorShaderModuleInfo> stages;
    };

    /// <summary>
    /// Creates an unified pipeline layout descriptor by merging one or more shader stage reflections.
    /// </summary>
    /// <param name="info"></param>
    /// <param name="descriptor"></param>
    /// <returns></returns>
    MergeShaderReflectionResult createPipelineLayoutDescriptor(PipelineLayoutDescriptorCreateInfo const& info, PipelineLayoutDescriptor& descriptor) noexcept;
}

// Provide custom hashing to use with std::unordered_map
namespace std
{
    template<>
    struct hash<litl::vulkan::DescriptorSetLayoutBindingDesc>
    {
        size_t operator()(litl::vulkan::DescriptorSetLayoutBindingDesc const& binding) const noexcept
        {
            return litl::hashPOD(binding);
        }
    };

    template<>
    struct hash<litl::vulkan::DescriptorSetLayoutDesc>
    {
        size_t operator()(litl::vulkan::DescriptorSetLayoutDesc const& layout) const noexcept
        {
            std::size_t h = 0ll;

            litl::hashCombine64(h, layout.bindings.size());
            for (auto const& binding : layout.bindings) { litl::hashCombine64(h, std::hash<litl::vulkan::DescriptorSetLayoutBindingDesc>{}(binding)); }

            return h;
        }
    };

    template<>
    struct hash<litl::vulkan::DescriptorSetLayoutCacheKey>
    {
        size_t operator()(litl::vulkan::DescriptorSetLayoutCacheKey const& key) const noexcept
        {
            size_t h = std::hash<litl::vulkan::DescriptorSetLayoutDesc>{}(key.desc);

            // Layout has padding (thanks to the bool+uint32 shape of DescriptorSetLayoutOptions) so hash and combine the individual elements and dont hash the entire object.
            litl::hashCombine64(h, litl::hashPOD(key.options.capacities));
            litl::hashCombine64(h, static_cast<uint64_t>(key.options.isPushDescriptor ? 1u : 0u));

            return h;
        }
    };
}

#endif