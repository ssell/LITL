#ifndef LITL_RENDERER_VULKAN_SHADER_MODULE_REFERENCE_MAP_H__
#define LITL_RENDERER_VULKAN_SHADER_MODULE_REFERENCE_MAP_H__

#include <vector>

#include "litl-core/stringId.hpp"
#include "litl-renderer-vulkan/resources/computePipeline.hpp"
#include "litl-renderer-vulkan/resources/graphicsPipeline.hpp"
#include "litl-renderer-vulkan/resources/shaderModule.hpp"

namespace litl::vulkan
{
    class ResourceManager;

    /// <summary>
    /// Maintains mappings between Shader Modules and pipeliens that reference them.
    /// Used for shader hot reloading.
    /// </summary>
    class ShaderModuleReferenceMap
    {
    public:

        ShaderModuleReferenceMap();
        ~ShaderModuleReferenceMap();

        ShaderModuleReferenceMap(ShaderModuleReferenceMap const&) = delete;
        ShaderModuleReferenceMap& operator=(ShaderModuleReferenceMap const&) = delete;

        void onShaderModuleAdded(ShaderModuleResource* shader) noexcept;
        void onShaderModuleDestroyed(ShaderModuleResource* shader) noexcept;

        void onGraphicsPipelineAdded(ResourceManager* resources, GraphicsPipelineResource* pipeline) noexcept;
        void onGraphicsPipelineDestroyed(ResourceManager* resources, GraphicsPipelineResource* pipeline) noexcept;

        void onComputePipelineAdded(ResourceManager* resources, ComputePipelineResource* pipeline) noexcept;
        void onComputePipelineDestroyed(ResourceManager* resources, ComputePipelineResource* pipeline) noexcept;

        void getGraphicsPipelinesFor(ShaderModuleResource* shader, std::vector<GraphicsPipelineResource*>& pipelines) const noexcept;
        void getComputePipelinesFor(ShaderModuleResource* shader, std::vector<ComputePipelineResource*>& pipelines) const noexcept;

    private:

        void mapGraphicsPipelineToShader(ShaderModuleResource* shader, GraphicsPipelineResource* pipeline) noexcept;
        void unmapGraphicsPipelineFromShader(ShaderModuleResource* shader, GraphicsPipelineResource* pipeline) noexcept;

        void mapComputePipelineToShader(ShaderModuleResource* shader, ComputePipelineResource* pipeline) noexcept;
        void unmapComputePipelineFromShader(ShaderModuleResource* shader, ComputePipelineResource* pipeline) noexcept;

        StringIdMap<std::vector<GraphicsPipelineResource*>> m_shaderToGraphicsPipelineMap;
        StringIdMap<std::vector<ComputePipelineResource*>> m_shaderToComputePipelineMap;
    };
}

#endif