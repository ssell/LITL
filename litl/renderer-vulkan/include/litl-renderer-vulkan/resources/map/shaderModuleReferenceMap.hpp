#ifndef LITL_RENDERER_VULKAN_SHADER_MODULE_REFERENCE_MAP_H__
#define LITL_RENDERER_VULKAN_SHADER_MODULE_REFERENCE_MAP_H__

#include <vector>

namespace litl::vulkan
{
    struct ShaderModuleResource;
    struct GraphicsPipelineResource;
    struct ComputePipelineResource;

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

        void onShaderModuleAdded(ShaderModuleResource* resource) noexcept;
        void onShaderModuleDestroyed(ShaderModuleResource* resource) noexcept;

        void onGraphicsPipelineAdded(GraphicsPipelineResource* resource) noexcept;
        void onGraphicsPipelineDestroyed(GraphicsPipelineResource* resource) noexcept;

        void onComputePipelineAdded(ComputePipelineResource* resource) noexcept;
        void onComputePipelineDestroyed(ComputePipelineResource* resource) noexcept;

        void getGraphicsPipelinesFor(ShaderModuleResource* resource, std::vector<GraphicsPipelineResource*>& pipelines) const noexcept;
        void getComputePipelinesFor(ShaderModuleResource* resource, std::vector<ComputePipelineResource*>& pipelines) const noexcept;

    private:
    };
}

#endif