#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/materialConstants.hpp"
#include "litl-engine/objects/material/materialManager.hpp"
#include "litl-engine/objects/material/deferredMaterialCommands.hpp"

namespace litl
{
    void MaterialManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_pObjectPool = services.get<ObjectPool>();
        LITL_FATAL_ASSERT_MSG((m_pObjectPool != nullptr), "Failed to inject ObjectPool into MaterialManager");
    }

    void MaterialManager::onFrameStart(Authority<EngineCallbacks> auth, uint32_t ecsFrame, uint32_t renderFrameIndex) noexcept
    {
        m_frame = ecsFrame;     // We use the ECS frame, not the Render frame, since frame staleness is based off of the ActiveMaterialSystem which uses the ECS frame count. ECS frame does not necessarily equal Render frame.
        m_pObjectPool->getAllMaterialHandles(m_materialHandles);

        for (auto& materialHandle : m_materialHandles)
        {
            auto* material = m_pObjectPool->getMaterial(materialHandle);

            if (material != nullptr)
            {
                material->onFrameStart({}, ecsFrame, renderFrameIndex);
            }
        }
    }

    void MaterialManager::onPreRender(Authority<EngineCallbacks> auth) noexcept
    {
        // Apply any waiting deferred material commands
        DeferredMaterialCommands::onPreRender({}, *m_pObjectPool);

        // Instruct each material to perform their onPreRender actions.
        m_pObjectPool->getAllMaterialHandles(m_materialHandles);

        for (auto& materialHandle : m_materialHandles)
        {
            auto* material = m_pObjectPool->getMaterial(materialHandle);

            if (material != nullptr)
            {
                // Free unused slots, updates frequent blocks, updated material buffers, etc.
                material->onPreRender({});
            }
        }

        // Instruct each multi-material bindings to clean up any unused bindings.
        m_pObjectPool->getAllMaterialBindingsHandles(m_materialBindingsHandles);

        for (auto& materialBindingsHandle : m_materialBindingsHandles)
        {
            auto* materialBindings = m_pObjectPool->getMaterialBindings(materialBindingsHandle);

            if (materialBindings != nullptr)
            {
                const uint32_t lastActiveFrame = materialBindings->getLastActiveFrame();
                const uint32_t expirationFrame = lastActiveFrame + MaterialBindingsExpirationFrames;

                if ((lastActiveFrame != 0u) && (expirationFrame < m_frame))
                {
                    m_pObjectPool->destroyMaterialBindings(materialBindingsHandle);
                }
            }
        }
    }
}