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
    /// What each set within a DescriptorSetLayout correlates to.
    /// </summary>
    enum class DescriptorSetIndex : uint32_t
    {
        /// <summary>
        /// Descriptor set that is consistent the entire frame.
        /// Example: (view matrix, projection matrix, time, camera position, frame uniforms)
        /// </summary>
        PerFrame = 0,

        /// <summary>
        /// Descriptor set that is consistent for an entire render pass.
        /// Example: opaque = (shadow maps, environment data) or transparent = (depth buffer)
        /// </summary>
        PerPass = 1,

        /// <summary>
        /// Descriptor set shared by all objects using the same material.
        /// Example: skybox = (cloud texture) or opaque = (material textures) or transparent = (smoke texture, noise texture)
        /// </summary>
        PerMaterial = 2,

        /// <summary>
        /// Descriptor set specific to each object though the layout may be implicitly shared by other objects.
        /// Example: (object index, instance data)
        /// </summary>
        PerObject = 3
    };

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
        std::string entryPoint;
    };

    /// <summary>
    /// Info used to merge reflected shader data into a single PipelineLayoutDescriptor.
    /// </summary>
    struct PipelineLayoutDescriptorCreateInfo
    {
        PipelineLayoutDescriptorShaderModuleInfo vertex;
        PipelineLayoutDescriptorShaderModuleInfo fragment;
        PipelineLayoutDescriptorShaderModuleInfo geometry;
        PipelineLayoutDescriptorShaderModuleInfo tessellationControl;
        PipelineLayoutDescriptorShaderModuleInfo tessellationEvaluation;
        //PipelineLayoutDescriptorShaderModuleInfo compute;
        //PipelineLayoutDescriptorShaderModuleInfo mesh;
        //PipelineLayoutDescriptorShaderModuleInfo task;
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
        std::size_t operator()(litl::vulkan::DescriptorSetLayoutBindingDesc const& binding) const noexcept
        {
            return litl::hashPOD(binding);
        }
    };

    template<>
    struct hash<litl::vulkan::DescriptorSetLayoutDesc>
    {
        std::size_t operator()(litl::vulkan::DescriptorSetLayoutDesc const& layout) const noexcept
        {
            std::size_t h = 0;

            litl::hashCombine64(h, layout.bindings.size());
            for (auto const& binding : layout.bindings) { litl::hashCombine64(h, std::hash<litl::vulkan::DescriptorSetLayoutBindingDesc>{}(binding)); }

            return h;
        }
    };
}

#endif