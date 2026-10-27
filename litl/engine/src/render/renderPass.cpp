#include "litl-core/assert.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/render/renderPass.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    void RenderPass::setup(Renderer& renderer, ObjectPool& objectPool) noexcept
    {
        m_pRenderer = &renderer;
        m_pObjectPool = &objectPool;
    }

    [[nodiscard]] bool RenderPass::compileRenderList(std::span<RenderableEntity const> entities) noexcept
    {
        m_renderPassItems.clear();

        if (entities.empty())
        {
            return false;
        }

        std::optional<MeshHandle> currMeshHandle{ std::nullopt };
        std::optional<MaterialHandle> currMaterialHandle{ std::nullopt };
        std::optional<uint32_t> currFirstIndex{ std::nullopt };
        std::optional<uint32_t> currIndexCount{ std::nullopt };

        //std::optional<MaterialBindingsHandle> currMaterialBindingsHandle{ std::nullopt };

        for (uint32_t i = 0u; i < static_cast<uint32_t>(entities.size()); ++i)
        {
            auto& renderableEntity = entities[i];

            if (currMeshHandle.has_value() && (*currMeshHandle == renderableEntity.meshRef.handle) &&
                currMaterialHandle.has_value() && (*currMaterialHandle == renderableEntity.materialRef.handle) &&
                currFirstIndex.has_value() && (*currFirstIndex == renderableEntity.firstIndex) &&
                currIndexCount.has_value() && (*currIndexCount == renderableEntity.indexCount))
            {
                // Same bound (sub)mesh and material(s)
                continue;
            }

            if (!m_renderPassItems.empty())
            {
                m_renderPassItems.back().instanceCount = i - m_renderPassItems.back().instanceOffset;
            }

            if (createRenderPassItems(renderableEntity, i, m_renderPassItems, currMeshHandle, currMaterialHandle))
            {
                currMeshHandle = renderableEntity.meshRef.handle;
                currMaterialHandle = renderableEntity.materialRef.handle;
                currFirstIndex = renderableEntity.firstIndex;
                currIndexCount = renderableEntity.indexCount;
            }
        }

        if (m_renderPassItems.empty())
        {
            return false;
        }

        m_renderPassItems.back().instanceCount = static_cast<uint32_t>(entities.size()) - m_renderPassItems.back().instanceOffset;    // Update instance count for the last drawable item
        
        return true;
    }

    [[nodiscard]] bool RenderPass::createRenderPassItems(RenderableEntity entity, uint32_t instanceOffset, std::vector<RenderPassItem>& m_renderPassItems, std::optional<MeshHandle>& currMeshHandle, std::optional<MaterialHandle>& currMaterialHandle) noexcept
    {
        auto* mesh = m_pObjectPool->getMesh(entity.meshRef.handle);

        if (mesh == nullptr)
        {
            return false;
        }

        auto& meshDescriptor = mesh->getDescriptor();

        if (entity.firstIndex >= meshDescriptor.indexInfo.indexCount)
        {
            return false;
        }

        auto* material = m_pObjectPool->getMaterial(entity.materialRef.handle);

        if (material == nullptr)
        {
            return false;
        }

        m_renderPassItems.push_back(RenderPassItem{
            .materialHandle = material->getHandle(),
            .material = material,
            .graphicsPipelineHandle = material->getGraphicsPipelineHandle(),
            .meshHandle = entity.meshRef.handle,
            .mesh = mesh,
            .firstVertex = 0u,
            .vertexCount = meshDescriptor.vertexInfo.vertexCount,
            .firstIndex = entity.firstIndex,
            .indexCount = litl::min(entity.indexCount, meshDescriptor.indexInfo.indexCount - entity.firstIndex),
            .instanceCount = 0u,
            .instanceOffset = instanceOffset
            });

        // Only update current handles on successful object retrievals and subsequent DrawListItem creation.
        return true;
    }
}