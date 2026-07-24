#include "litl-core/authority.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/render/renderManager.hpp"

namespace litl
{
    void EngineCallbacks::setup(Authority<Engine> authority, ServiceProvider& services, std::shared_ptr<FrameCallbacks> userCallbacks) noexcept
    {
        m_pWorld = services.get<World>();
        m_pSceneManager = services.get<SceneManager>();
        m_pRenderManager = services.get<RenderManager>();
        m_pUserFrameCallbacks = (userCallbacks != nullptr) ? userCallbacks : std::make_shared<FrameCallbacks>();

        LITL_FATAL_ASSERT_MSG((m_pWorld != nullptr), "Failed to inject World into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG((m_pSceneManager != nullptr), "Failed to inject SceneManager into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG((m_pRenderManager != nullptr), "Failed to inject RenderManager into EngineCallbacks");

        // ---------------------------------------------------------------------------------
        // Intraframe Sync Points
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onSyncPoint = [this](ServiceProvider& services, SystemGroup group, std::span<EntityChange const> entityChanges)
            {
                m_pSceneManager->processEntityChanges(Authority<EngineCallbacks>{}, * m_pWorld, entityChanges);
                m_pUserFrameCallbacks->invokeSyncPoint(services, group, entityChanges);
            };

        // ---------------------------------------------------------------------------------
        // Frame Start
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onFrameStart = [this](ServiceProvider& services, float dt)
            {
                m_pUserFrameCallbacks->invokeFrameStart(services, dt);
            };

        // ---------------------------------------------------------------------------------
        // Startup Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Startup)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Input Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Input)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Fixed Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::FixedUpdate)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Update)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Late Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::LateUpdate)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Pre-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PreRender)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pSceneManager->onPreRender(Authority<EngineCallbacks>{}, *m_pWorld);
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Render
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onRender = [this](ServiceProvider& services, float dt)
            {
                m_pRenderManager->onRender({}, dt);
                m_pUserFrameCallbacks->invokeRender(services, dt);
            };

        // ---------------------------------------------------------------------------------
        // Post-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PostRender)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Final Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Final)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Frame End
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onFrameEnd = [this](ServiceProvider& services, float dt)
            {
                m_pUserFrameCallbacks->invokeFrameEnd(services, dt);
            };
    }

    std::shared_ptr<FrameCallbacks> EngineCallbacks::getFrameCallbacks() noexcept
    {
        return m_pFrameCallbacks;
    }
}