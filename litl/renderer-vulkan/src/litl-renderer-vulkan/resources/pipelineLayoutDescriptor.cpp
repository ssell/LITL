#include <algorithm>
#include <optional>
#include <ranges>
#include <span>

#include "litl-core/constants.hpp"
#include "litl-renderer-vulkan/resources/shaderModule.hpp"
#include "litl-renderer-vulkan/resources/pipelineLayoutDescriptor.hpp"

namespace litl::vulkan
{
    namespace
    {
        struct SetBindingKey
        {
            uint32_t set = 0;
            uint32_t binding = 0;

            bool operator==(SetBindingKey const&) const = default;
        };
    }

    /// <summary>
    /// Adds the resource to the descriptor if it is not yet present.
    /// Otherwise, if a resource already exists with the same set/binding it attempts to "merge" them together.
    /// The merge is successful if the existing resource and the new resource match on type and array size.
    /// Otherwise the appropriate error is returned.
    /// </summary>
    /// <param name="descriptor"></param>
    /// <param name="stage"></param>
    /// <param name="resource"></param>
    /// <returns></returns>
    MergeShaderReflectionResult addOrMergeResource(std::vector<MergedResourceBinding>& resources, ShaderStage stage, ResourceBinding const& resource) noexcept
    {
        std::optional<size_t> index = std::nullopt;

        // Find existing resource
        for (size_t i = 0; i < resources.size(); ++i)
        {
            if ((resources[i].set == resource.set) &&
                (resources[i].binding == resource.binding))
            {
                index = i;
                break;
            }
        }

        if (!index.has_value())
        {
            // New resource
            resources.push_back(MergedResourceBinding{
                .type = resource.type,
                .set = resource.set,
                .binding = resource.binding,
                .arraySize = resource.arraySize,
                .sizeBytes = resource.sizeBytes,
                .stages = stage
            });
        }
        else
        {
            // Already exists, try to merge.
            auto& existingResource = resources[index.value()];

            if (existingResource.type != resource.type)
            {
                return MergeShaderReflectionResult::ErrorTypeMismatch;
            }
            else if (existingResource.arraySize != resource.arraySize)
            {
                return MergeShaderReflectionResult::ErrorArraySizeMismatch;
            }
            else if (existingResource.sizeBytes != resource.sizeBytes)
            {
                return MergeShaderReflectionResult::ErrorBufferSizeMismatch;
            }

            existingResource.stages = existingResource.stages | stage;
        }

        return MergeShaderReflectionResult::Success;
    }

    /// <summary>
    /// Adds the push constant to the descriptor if it is not yet present.
    /// If a push constant already exists with the same offset/size then it is updated to also be present in the current stage.
    /// Otherwise, if the range is currently unused then the push constant is added. If the range is already occupied then 
    /// the appropriate error is returned.
    /// </summary>
    /// <param name="descriptor"></param>
    /// <param name="stage"></param>
    /// <param name="pushConstant"></param>
    /// <returns></returns>
    MergeShaderReflectionResult addOrMergePushConstant(std::vector<MergedPushConstantRange>& pushConstants, ShaderStage stage, PushConstantRange const& pushConstant) noexcept
    {
        size_t index = Constants::uint32_null_index;

        // Find existing push constant
        for (size_t i = 0; i < pushConstants.size(); ++i)
        {
            if ((pushConstants[i].offset == pushConstant.offset) &&
                (pushConstants[i].sizeBytes == pushConstant.sizeBytes))
            {
                index = i;
                break;
            }
        }

        if (index == Constants::uint32_null_index)
        {
            // Check if the range is already occupied
            bool overlap = std::ranges::any_of(pushConstants,
                [&](MergedPushConstantRange const& mpc)
                {
                    uint32_t aStart = pushConstant.offset;
                    uint32_t aEnd = aStart + pushConstant.sizeBytes;

                    uint32_t bStart = mpc.offset;
                    uint32_t bEnd = bStart + mpc.sizeBytes;

                    // True if the two regions overlap
                    return (aStart < bEnd) && (bStart < aEnd);
                });

            if (overlap)
            {
                return MergeShaderReflectionResult::ErrorPushConstantOverlap;
            }

            // New push constant
            pushConstants.push_back(MergedPushConstantRange{
                .offset = pushConstant.offset,
                .sizeBytes = pushConstant.sizeBytes,
                .stages = stage
                });
        }
        else
        {
            // Already exists
            pushConstants[index].stages = pushConstants[index].stages | stage;
        }

        return MergeShaderReflectionResult::Success;
    }

    /// <summary>
    /// Entering this function we have a flat list of resource bindings that describe a resource by its type,
    /// array size, shader stage, and which set it is in. We transform the flat list into a hierarchical structure
    /// to match the structure expected by Vulkan.
    /// </summary>
    /// <param name="descriptor"></param>
    /// <param name="resourceBindings"></param>
    void sortAndAssignMergedResources(PipelineLayoutDescriptor& descriptor, std::span<MergedResourceBinding const> resourceBindings) noexcept
    {
        descriptor.setLayouts.clear();

        // Partition our flat list of resource bindings into structured sets of bindings to mimic the Vulkan structure
        uint32_t maxSet = 0u;

        for (auto const& mergedResource : resourceBindings)
        {
            maxSet = std::max(maxSet, mergedResource.set);
        }

        descriptor.setLayouts.resize(resourceBindings.empty() ? 0 : maxSet + 1);

        for (auto const& mergedResource : resourceBindings)
        {
            descriptor.setLayouts[mergedResource.set].bindings.push_back(
                DescriptorSetLayoutBindingDesc{
                    .binding = mergedResource.binding,
                    .type = mergedResource.type,
                    .arraySize = mergedResource.arraySize,
                    .stages = mergedResource.stages
                }
            );
        }

        // Sort by the DescriptorSetLayoutDesc bindings as the Shader Reflection does not necessarily visit them in order. This is done for consistent hashing.
        for (auto& setLayout : descriptor.setLayouts)
        {
            if (setLayout.bindings.size() > 0)
            {
                std::sort(setLayout.bindings.begin(), setLayout.bindings.end(), [](DescriptorSetLayoutBindingDesc const& a, DescriptorSetLayoutBindingDesc const& b) -> bool
                    {
                        return a.binding < b.binding;
                    });
            }
        }
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="descriptor"></param>
    /// <param name="pushConstants"></param>
    void sortAndAssignPushConstants(PipelineLayoutDescriptor& descriptor, std::span<MergedPushConstantRange const> pushConstants)
    {
        descriptor.pushConstants.clear();
        descriptor.pushConstants.assign(pushConstants.begin(), pushConstants.end());

        if (descriptor.pushConstants.size() > 0)
        {
            std::sort(descriptor.pushConstants.begin(), descriptor.pushConstants.end(), [](MergedPushConstantRange const& a, MergedPushConstantRange const& b) -> bool
                {
                    return a.offset < b.offset;
                });
        }
    }

    /// <summary>
    /// Returns true if the shader stage has already been seen.
    /// </summary>
    /// <param name="seenStages"></param>
    /// <param name="stage"></param>
    /// <returns></returns>
    bool seenStage(ShaderStage& seenStages, ShaderStage stage) noexcept
    {
        return (static_cast<uint32_t>(seenStages) & static_cast<uint32_t>(stage)) != 0;
    }

    /// <summary>
    /// Given a info struct, attempts to retrieve the specified entry point and extract its reflection results.
    /// </summary>
    /// <param name="info"></param>
    /// <param name="seenStages"></param>
    /// <param name="resourceBindings"></param>
    /// <param name="pushConstants"></param>
    /// <param name="error"></param>
    /// <returns></returns>
    bool mergeShaderModuleResource(PipelineLayoutDescriptorShaderModuleInfo const& info, ShaderStage& seenStages, std::vector<MergedResourceBinding>& resourceBindings, std::vector<MergedPushConstantRange>& pushConstants, MergeShaderReflectionResult& error) noexcept
    {
        error = MergeShaderReflectionResult::Success;

        if (info.resource == nullptr)
        {
            // skip silenty
            return true;
        }

        if (info.stage == ShaderStage::None)
        {
            error = MergeShaderReflectionResult::ErrorInvalidShaderStage;
            return false;
        }

        if (seenStage(seenStages, info.stage))
        {
            error = MergeShaderReflectionResult::ErrorDuplicateStage;
            return false;
        }

        seenStages = seenStages | info.stage;

        auto entryPoint = info.resource->reflection.getEntryPoint(info.entryPoint);

        if (!entryPoint.has_value())
        {
            error = MergeShaderReflectionResult::ErrorInvalidEntryPoint;
            return false;
        }

        if (entryPoint.value()->stage != info.stage)
        {
            error = MergeShaderReflectionResult::ErrorStageMismatch;
            return false;
        }

        // Merge the reflected resources
        for (auto const& resource : entryPoint.value()->resources)
        {
            MergeShaderReflectionResult result = addOrMergeResource(resourceBindings, entryPoint.value()->stage, resource);

            if (result != MergeShaderReflectionResult::Success)
            {
                error = result;
                return false;
            }
        }

        // Merge the reflected push constants
        for (auto const& pushConstant : entryPoint.value()->pushConstants)
        {
            MergeShaderReflectionResult result = addOrMergePushConstant(pushConstants, entryPoint.value()->stage, pushConstant);

            if (result != MergeShaderReflectionResult::Success)
            {
                error = result;
                return false;
            }
        }
        
        return true;
    }

    MergeShaderReflectionResult createPipelineLayoutDescriptor(PipelineLayoutDescriptorCreateInfo const& info, PipelineLayoutDescriptor& descriptor) noexcept
    {
        MergeShaderReflectionResult result = MergeShaderReflectionResult::Success;

        // Accumulate the resource bindings and push constants for each shader
        ShaderStage seenStages = ShaderStage::None;

        std::vector<MergedResourceBinding> resourceBindings;
        std::vector<MergedPushConstantRange> pushConstants;

        for (auto const& stage : info.stages)
        {
            if (!mergeShaderModuleResource(stage, seenStages, resourceBindings, pushConstants, result))
            {
                return result;
            }
        }

        if (!seenStage(seenStages, ShaderStage::Vertex) && !(seenStage(seenStages, ShaderStage::Compute) || seenStage(seenStages, ShaderStage::Mesh)))
        {
            // If no vertex shader was seen AND neither a compute or mesh shader was seen then
            return MergeShaderReflectionResult::ErrorNoVertexStage;
        }

        sortAndAssignMergedResources(descriptor, resourceBindings);
        sortAndAssignPushConstants(descriptor, pushConstants);

        return result;
    }
}