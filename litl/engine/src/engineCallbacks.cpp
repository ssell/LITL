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

        m_pFrameCallbacks->onSyncPoint = [this](SystemGroup group, std::span<EntityChange const> entityChanges)
            {
                m_pSceneManager->processEntityChanges(Authority<EngineCallbacks>{}, * m_pWorld, entityChanges);
                m_pUserFrameCallbacks->invokeSyncPoint(group, entityChanges);
            };

        // ---------------------------------------------------------------------------------
        // Frame Start
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onFrameStart = [this]()
            {
                m_pUserFrameCallbacks->invokeFrameStart();
            };

        // ---------------------------------------------------------------------------------
        // Startup Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Startup)] = [this](SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Input Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Input)] = [this](SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Fixed Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::FixedUpdate)] = [this](SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Update)] = [this](SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Late Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::LateUpdate)] = [this](SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Pre-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PreRender)] = [this](SystemGroup group)
            {
                m_pSceneManager->onPreRender(Authority<EngineCallbacks>{}, *m_pWorld);
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Render
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onRender = [this]()
            {
                m_pRenderManager->onRender({});
                m_pUserFrameCallbacks->invokeRender();
            };

        // ---------------------------------------------------------------------------------
        // Post-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PostRender)] = [this](SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Final Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Final)] = [this](SystemGroup group)
            {
                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Frame End
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onFrameEnd = [this]()
            {
                m_pUserFrameCallbacks->invokeFrameEnd();
            };
    }

    std::shared_ptr<FrameCallbacks> EngineCallbacks::getFrameCallbacks() noexcept
    {
        return m_pFrameCallbacks;
    }
}