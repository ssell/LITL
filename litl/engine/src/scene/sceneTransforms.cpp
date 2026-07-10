#include "litl-engine/scene/sceneTransforms.hpp"
#include "litl-core/assert.hpp"

namespace litl
{
    void SceneTransforms::reserve(uint32_t entityCount) noexcept
    {
        if (entityCount > m_worldMatrices.size())
        {
            m_worldMatrices.reserve(entityCount);

            while (entityCount > m_worldMatrices.size())
            {
                m_worldMatrices.push_back({});
            }
        }
    }

    mat4 SceneTransforms::getWorldMatrix(uint32_t entityGpuIndex) const noexcept
    {
        LITL_ASSERT_MSG((entityGpuIndex < m_worldMatrices.size()), "Out-of-bounds index specified to SceneTransforms::getWorldMatrix", {});
        return m_worldMatrices[entityGpuIndex];
    }

    void SceneTransforms::setWorldMatrix(uint32_t entityGpuIndex, mat4 const& worldMatrix) noexcept
    {
        LITL_ASSERT_MSG((entityGpuIndex < m_worldMatrices.size()), "Out-of-bounds index specified to SceneTransforms::setWorldMatrix", );
        m_worldMatrices[entityGpuIndex] = worldMatrix;
    }

    std::span<mat4 const> SceneTransforms::getWorldMatrices() const noexcept
    {
        return m_worldMatrices;
    }
}