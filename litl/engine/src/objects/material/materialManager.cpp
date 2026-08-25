#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/materialManager.hpp"

namespace litl
{
    void MaterialManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_pObjectPool = services.get<ObjectPool>().get();
        LITL_FATAL_ASSERT_MSG((m_pObjectPool != nullptr), "Failed to inject ObjectPool into MaterialManager");
    }

    void MaterialManager::onFrameStart(Authority<EngineCallbacks> auth, uint32_t frame, uint32_t frameIndex) noexcept
    {
        m_pObjectPool->getAllMaterialHandles(m_materialHandles);

        for (auto& materialHandle : m_materialHandles)
        {
            auto* material = m_pObjectPool->getMaterial(materialHandle);

            if (material != nullptr)
            {
                material->onFrameStart({}, frame, frameIndex);
            }
        }
    }

    void MaterialManager::onPreRender(Authority<EngineCallbacks> auth) noexcept
    {
        m_pObjectPool->getAllMaterialHandles(m_materialHandles);

        for (auto& materialHandle : m_materialHandles)
        {
            auto* material = m_pObjectPool->getMaterial(materialHandle);

            if (material != nullptr)
            {
                material->onPreRender({});
            }
        }
    }
}