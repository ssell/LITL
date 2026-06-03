#include "litl-core/assert.hpp"
#include "litl-renderer-vulkan/resources/map/shaderModuleReferenceMap.hpp"
#include "litl-renderer-vulkan/resources/shaderModule.hpp"

namespace litl::vulkan
{
    ShaderModuleReferenceMap::ShaderModuleReferenceMap()
    {

    }

    ShaderModuleReferenceMap::~ShaderModuleReferenceMap()
    {

    }

    void ShaderModuleReferenceMap::onShaderModuleAdded(ShaderModuleResource* resource) noexcept
    {
        LITL_ASSERT_MSG(resource != nullptr, "ShaderModuleReferenceMap::onShaderModuleAdded provided NULL shader module resource", );
    }

    void ShaderModuleReferenceMap::onShaderModuleDestroyed(ShaderModuleResource* resource) noexcept
    {
        LITL_ASSERT_MSG(resource != nullptr, "ShaderModuleReferenceMap::onShaderModuleDestroyed provided NULL shader module resource", );
    }

    void ShaderModuleReferenceMap::onGraphicsPipelineAdded(GraphicsPipelineResource* resource) noexcept
    {

    }

    void ShaderModuleReferenceMap::onGraphicsPipelineDestroyed(GraphicsPipelineResource* resource) noexcept
    {

    }

    void ShaderModuleReferenceMap::onComputePipelineAdded(ComputePipelineResource* resource) noexcept
    {

    }

    void ShaderModuleReferenceMap::onComputePipelineDestroyed(ComputePipelineResource* resource) noexcept
    {

    }

    void ShaderModuleReferenceMap::getGraphicsPipelinesFor(ShaderModuleResource* resource, std::vector<GraphicsPipelineResource*>& pipelines) const noexcept
    {
        LITL_ASSERT_MSG(resource != nullptr, "ShaderModuleReferenceMap::getGraphicsPipelinesFor provided NULL shader module resource", );
    }

    void ShaderModuleReferenceMap::getComputePipelinesFor(ShaderModuleResource* resource, std::vector<ComputePipelineResource*>& pipelines) const noexcept
    {
        LITL_ASSERT_MSG(resource != nullptr, "ShaderModuleReferenceMap::getComputePipelinesFor provided NULL shader module resource", );
    }
}