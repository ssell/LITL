#include "litl-core/assert.hpp"
#include "litl-core/debug.hpp"

#include "litl-renderer-vulkan/resources/map/shaderModuleReferenceMap.hpp"
#include "litl-renderer-vulkan/resources/shaderModule.hpp"
#include "litl-renderer-vulkan/resourceManager.hpp"

namespace litl::vulkan
{
    ShaderModuleReferenceMap::ShaderModuleReferenceMap()
    {

    }

    ShaderModuleReferenceMap::~ShaderModuleReferenceMap()
    {

    }

    void ShaderModuleReferenceMap::onShaderModuleAdded(ShaderModuleResource* shader) noexcept
    {
        LITL_ASSERT_MSG(shader != nullptr, "ShaderModuleReferenceMap::onShaderModuleAdded provided NULL shader module resource", );
        LITL_ASSERT_MSG((m_shaderToGraphicsPipelineMap.find(shader->resourceId) == m_shaderToGraphicsPipelineMap.end()) || m_shaderToComputePipelineMap.find(shader->resourceId) == m_shaderToComputePipelineMap.end(), "ShaderModuleReferenceMap::onShaderModuleAdded provided duplicate shader module resource", );

        m_shaderToGraphicsPipelineMap[shader->resourceId] = {};
        m_shaderToComputePipelineMap[shader->resourceId] = {};
    }

    void ShaderModuleReferenceMap::onShaderModuleDestroyed(ShaderModuleResource* shader) noexcept
    {
        LITL_ASSERT_MSG(shader != nullptr, "ShaderModuleReferenceMap::onShaderModuleDestroyed provided NULL shader module resource", );

        m_shaderToGraphicsPipelineMap.erase(shader->resourceId);
        m_shaderToComputePipelineMap.erase(shader->resourceId);
    }

    void ShaderModuleReferenceMap::onGraphicsPipelineAdded(ResourceManager* resources, GraphicsPipelineResource* pipeline) noexcept
    {
        LITL_ASSERT_MSG(pipeline != nullptr, "ShaderModuleReferenceMap::onGraphicsPipelineAdded provided NULL graphics pipeline resource", );

        mapGraphicsPipelineToShader(resources->getShaderModule(pipeline->descriptor.vertex.handle), pipeline);
        mapGraphicsPipelineToShader(resources->getShaderModule(pipeline->descriptor.fragment.handle), pipeline);
        mapGraphicsPipelineToShader(resources->getShaderModule(pipeline->descriptor.geometry.handle), pipeline);
        mapGraphicsPipelineToShader(resources->getShaderModule(pipeline->descriptor.tessellationControl.handle), pipeline);
        mapGraphicsPipelineToShader(resources->getShaderModule(pipeline->descriptor.tessellationEvaluation.handle), pipeline);
        mapGraphicsPipelineToShader(resources->getShaderModule(pipeline->descriptor.mesh.handle), pipeline);
        mapGraphicsPipelineToShader(resources->getShaderModule(pipeline->descriptor.task.handle), pipeline);
    }

    void ShaderModuleReferenceMap::onGraphicsPipelineDestroyed(ResourceManager* resources, GraphicsPipelineResource* pipeline) noexcept
    {
        LITL_ASSERT_MSG(pipeline != nullptr, "ShaderModuleReferenceMap::onGraphicsPipelineDestroyed provided NULL graphics pipeline resource", );

        unmapGraphicsPipelineFromShader(resources->getShaderModule(pipeline->descriptor.vertex.handle), pipeline);
        unmapGraphicsPipelineFromShader(resources->getShaderModule(pipeline->descriptor.fragment.handle), pipeline);
        unmapGraphicsPipelineFromShader(resources->getShaderModule(pipeline->descriptor.geometry.handle), pipeline);
        unmapGraphicsPipelineFromShader(resources->getShaderModule(pipeline->descriptor.tessellationControl.handle), pipeline);
        unmapGraphicsPipelineFromShader(resources->getShaderModule(pipeline->descriptor.tessellationEvaluation.handle), pipeline);
        unmapGraphicsPipelineFromShader(resources->getShaderModule(pipeline->descriptor.mesh.handle), pipeline);
        unmapGraphicsPipelineFromShader(resources->getShaderModule(pipeline->descriptor.task.handle), pipeline);
    }

    void ShaderModuleReferenceMap::onComputePipelineAdded(ResourceManager* resources, ComputePipelineResource* pipeline) noexcept
    {
        LITL_ASSERT_MSG(pipeline != nullptr, "ShaderModuleReferenceMap::onComputePipelineAdded provided NULL graphics pipeline resource", );
        
        mapComputePipelineToShader(resources->getShaderModule(pipeline->descriptor.compute.handle), pipeline);
    }

    void ShaderModuleReferenceMap::onComputePipelineDestroyed(ResourceManager* resources, ComputePipelineResource* pipeline) noexcept
    {
        LITL_ASSERT_MSG(pipeline != nullptr, "ShaderModuleReferenceMap::onComputePipelineDestroyed provided NULL graphics pipeline resource", );
        
        unmapComputePipelineFromShader(resources->getShaderModule(pipeline->descriptor.compute.handle), pipeline);
    }

    void ShaderModuleReferenceMap::getGraphicsPipelinesFor(ShaderModuleResource* shader, std::vector<GraphicsPipelineResource*>& pipelines) const noexcept
    {
        LITL_ASSERT_MSG(shader != nullptr, "ShaderModuleReferenceMap::getGraphicsPipelinesFor provided NULL shader module resource", );

        if (auto find = m_shaderToGraphicsPipelineMap.find(shader->resourceId); find != m_shaderToGraphicsPipelineMap.end())
        {
            pipelines.assign(find->second.begin(), find->second.end());
        }
    }

    void ShaderModuleReferenceMap::getComputePipelinesFor(ShaderModuleResource* shader, std::vector<ComputePipelineResource*>& pipelines) const noexcept
    {
        LITL_ASSERT_MSG(shader != nullptr, "ShaderModuleReferenceMap::getComputePipelinesFor provided NULL shader module resource", );

        if (auto find = m_shaderToComputePipelineMap.find(shader->resourceId); find != m_shaderToComputePipelineMap.end())
        {
            pipelines.assign(find->second.begin(), find->second.end());
        }
    }

    void ShaderModuleReferenceMap::mapGraphicsPipelineToShader(ShaderModuleResource* shader, GraphicsPipelineResource* pipeline) noexcept
    {
        // onGraphicsPipelineAdded lazily calls this method, so a nullptr is an expected possibility
        if (shader == nullptr)
        {
            return;
        }

        auto find = m_shaderToGraphicsPipelineMap.find(shader->resourceId);

        LITL_ASSERT_MSG(find != m_shaderToGraphicsPipelineMap.end(), "ShaderModuleReferenceMap::mapGraphicsPipelineToShader provided unknown shader resource", );
    
        if constexpr (LITL_DEBUG)
        {
            // Make sure the pipeline isn't already mapped.
            for (auto* trackedPipeline : find->second)
            {
                LITL_ASSERT_MSG(pipeline != trackedPipeline, "ShaderModuleReferenceMap::mapGraphicsPipelineToShader provided already tracked graphics pipeline", );
                std::ignore = trackedPipeline; // get rid of the "unused variable" warning
            }
        }

        find->second.push_back(pipeline);
    }

    void ShaderModuleReferenceMap::unmapGraphicsPipelineFromShader(ShaderModuleResource* shader, GraphicsPipelineResource* pipeline) noexcept
    {
        // onGraphicsPipelineDestroyed lazily calls this method, so a nullptr is an expected possibility
        if (shader == nullptr)
        {
            return;
        }

        auto find = m_shaderToGraphicsPipelineMap.find(shader->resourceId);

        LITL_ASSERT_MSG(find != m_shaderToGraphicsPipelineMap.end(), "ShaderModuleReferenceMap::unmapGraphicsPipelineFromShader provided unknown shader resource", );
        
        size_t atIndex = 0;

        for (; atIndex < find->second.size(); ++atIndex)
        {
            if (find->second[atIndex] == pipeline)
            {
                break;
            }
        }

        LITL_ASSERT_MSG(atIndex != find->second.size(), "ShaderModuleReferenceMap::unmapGraphicsPipelineFromShader provided unmapped graphics pipeline", );

        find->second[atIndex] = find->second.back();
        find->second.pop_back();
    }

    void ShaderModuleReferenceMap::mapComputePipelineToShader(ShaderModuleResource* shader, ComputePipelineResource* pipeline) noexcept
    {
        // onComputePipelineAdded lazily calls this method, so a nullptr is an expected possibility
        if (shader == nullptr)
        {
            return;
        }

        auto find = m_shaderToComputePipelineMap.find(shader->resourceId);

        LITL_ASSERT_MSG(find != m_shaderToComputePipelineMap.end(), "ShaderModuleReferenceMap::mapComputePipelineToShader provided unknown shader resource", );

        if constexpr (LITL_DEBUG)
        {
            // Make sure the pipeline isn't already mapped.
            for (auto* trackedPipeline : find->second)
            {
                LITL_ASSERT_MSG(pipeline != trackedPipeline, "ShaderModuleReferenceMap::mapComputePipelineToShader provided already tracked compute pipeline", );
                std::ignore = trackedPipeline; // get rid of the "unused variable" warning
            }
        }

        find->second.push_back(pipeline);
    }

    void ShaderModuleReferenceMap::unmapComputePipelineFromShader(ShaderModuleResource* shader, ComputePipelineResource* pipeline) noexcept
    {
        // onComputePipelineDestroyed lazily calls this method, so a nullptr is an expected possibility
        if (shader == nullptr)
        {
            return;
        }

        auto find = m_shaderToComputePipelineMap.find(shader->resourceId);

        LITL_ASSERT_MSG(find != m_shaderToComputePipelineMap.end(), "ShaderModuleReferenceMap::unmapComputePipelineFromShader provided unknown shader resource", );

        size_t atIndex = 0;

        for (; atIndex < find->second.size(); ++atIndex)
        {
            if (find->second[atIndex] == pipeline)
            {
                break;
            }
        }

        LITL_ASSERT_MSG(atIndex != find->second.size(), "ShaderModuleReferenceMap::unmapComputePipelineFromShader provided unmapped compute pipeline", );

        find->second[atIndex] = find->second.back();
        find->second.pop_back();
    }

}