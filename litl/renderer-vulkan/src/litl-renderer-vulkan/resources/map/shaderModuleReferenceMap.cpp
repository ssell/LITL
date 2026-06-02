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

    void ShaderModuleReferenceMap::getPipelinesFor(ShaderModuleResource* resource, std::vector<UnifiedPipelineHandle>& pipelines) const noexcept
    {
        LITL_ASSERT_MSG(resource != nullptr, "ShaderModuleReferenceMap::getPipelinesFor provided NULL shader module resource", );
    }
}