#ifndef LITL_RENDERER_PIPELINE_LAYOUT_DESCRIPTOR_H__
#define LITL_RENDERER_PIPELINE_LAYOUT_DESCRIPTOR_H__

#include <vector>
#include "litl-renderer/pipeline/shaderReflectionMerge.hpp"

namespace litl
{
    /// <summary>
    /// Configuration for the pipeline layout.
    /// </summary>
    struct PipelineLayoutDescriptor
    {
        std::vector<MergedResourceBinding> resources;
        std::vector<MergedPushConstantRange> pushConstants;

        [[nodiscard]] uint64_t hash() const noexcept
        {
            if (m_hash == 0)
            {
                // do the hashing ...
            }

            return m_hash;
        }

    private:

        uint64_t m_hash = 0;
    };

    /// <summary>
    /// Outputs an unified pipeline layout descriptor from one or more shader stage reflections.
    /// </summary>
    /// <param name="reflectedShaderStages"></param>
    /// <param name="descriptor"></param>
    /// <returns></returns>
    MergeShaderReflectionResult mergeShaderReflections(std::span<ShaderReflection const> reflectedShaderStages, PipelineLayoutDescriptor& descriptor) noexcept;
}

#endif