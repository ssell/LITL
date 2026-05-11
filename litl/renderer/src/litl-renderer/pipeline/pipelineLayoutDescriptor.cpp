#include <algorithm>
#include <ranges>

#include "litl-core/constants.hpp"
#include "litl-renderer/pipeline/pipelineLayoutDescriptor.hpp"

namespace litl
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
    MergeShaderReflectionResult addOrMergeResource(PipelineLayoutDescriptor& descriptor, ShaderStage stage, ResourceBinding const& resource) noexcept
    {
        size_t index = Constants::uint32_null_index;

        // Find existing resource
        for (size_t i = 0; i < descriptor.resources.size(); ++i)
        {
            if ((descriptor.resources[i].set == resource.set) &&
                (descriptor.resources[i].binding == resource.binding))
            {
                index = i;
                break;
            }
        }

        if (index == Constants::uint32_null_index)
        {
            // New resource
            descriptor.resources.push_back(MergedResourceBinding{
                .type = resource.type,
                .set = resource.set,
                .binding = resource.binding,
                .arraySize = resource.arraySize,
                .stages = stage
            });
        }
        else
        {
            // Already exists, try to merge.
            auto& existingResource = descriptor.resources[index];

            if (existingResource.type != resource.type)
            {
                return MergeShaderReflectionResult::ErrorTypeMismatch;
            }
            else if (existingResource.arraySize != resource.arraySize)
            {
                return MergeShaderReflectionResult::ErrorArraySizeMismatch;
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
    MergeShaderReflectionResult addOrMergePushConstant(PipelineLayoutDescriptor& descriptor, ShaderStage stage, PushConstantRange const& pushConstant) noexcept
    {
        size_t index = Constants::uint32_null_index;

        // Find existing push constant
        for (size_t i = 0; i < descriptor.resources.size(); ++i)
        {
            if ((descriptor.pushConstants[i].offset == pushConstant.offset) &&
                (descriptor.pushConstants[i].sizeBytes == pushConstant.sizeBytes))
            {
                index = i;
                break;
            }
        }

        if (index == Constants::uint32_null_index)
        {
            // Check if the range is already occupied
            bool overlap = std::ranges::any_of(descriptor.pushConstants,
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
            descriptor.pushConstants.push_back(MergedPushConstantRange{
                .offset = pushConstant.offset,
                .sizeBytes = pushConstant.sizeBytes,
                .stages = stage
            });
        }
        else
        {
            // Already exists
            descriptor.pushConstants[index].stages = descriptor.pushConstants[index].stages | stage;
        }

        return MergeShaderReflectionResult::Success;
    }

    MergeShaderReflectionResult mergeShaderReflections(std::span<ShaderReflection const> reflectedShaderStages, PipelineLayoutDescriptor& descriptor) noexcept
    {
        auto result = MergeShaderReflectionResult::Success;

        // Keep track of already seen stages to catch duplicates.
        ShaderStage seenStages = ShaderStage::Unknown;

        // For each reflected shader stage (vertex, fragment, etc.) ...
        for (auto const& reflectedShaderStage : reflectedShaderStages)
        {
            if ((static_cast<uint32_t>(seenStages) & static_cast<uint32_t>(reflectedShaderStage.stage)) != 0)
            {
                return MergeShaderReflectionResult::ErrorDuplicateStage;
            }

            seenStages = seenStages | reflectedShaderStage.stage;

            // Merge the reflected resources
            for (auto const& resource : reflectedShaderStage.resources)
            {
                result = addOrMergeResource(descriptor, reflectedShaderStage.stage, resource);

                if (result != MergeShaderReflectionResult::Success)
                {
                    return result;
                }
            }

            // Merge the reflected push constants
            for (auto const& pushConstant : reflectedShaderStage.pushConstants)
            {
                result = addOrMergePushConstant(descriptor, reflectedShaderStage.stage, pushConstant);

                if (result != MergeShaderReflectionResult::Success)
                {
                    return result;
                }
            }
        }


        return result;
    }
}