#include "litl-engine/objects/material.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    bool Material::create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer, ObjectPool& objectPool) noexcept
    {
        m_pRenderer = &renderer;
        m_materialPipelineHandle = objectPool.createMaterialPipeline(descriptor.pipelineDescriptor);

        return true;
    }

    void Material::destroy(Authority<ObjectPool> auth) noexcept
    {

    }

    MaterialPipelineHandle Material::getMaterialPipelineHandle() const noexcept
    {
        return m_materialPipelineHandle;
    }
}