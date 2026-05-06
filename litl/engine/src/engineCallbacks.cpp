#include "litl-engine/engineCallbacks.hpp"

namespace litl
{
    void EngineCallbacks::setup(std::shared_ptr<ServiceProvider> serviceProvider, std::shared_ptr<FrameCallbacks> userCallbacks) noexcept
    {
        m_pWorld = serviceProvider->get<World>();
        m_pSceneManager = serviceProvider->get<SceneManager>();
        m_pUserFrameCallbacks = (userCallbacks != nullptr) ? userCallbacks : std::make_shared<FrameCallbacks>();

        LITL_FATAL_ASSERT_MSG(m_pWorld != nullptr, "Failed to inject litl::World into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG(m_pSceneManager != nullptr, "Failed to inject litl::SceneManager into EngineCallbacks");

        m_pFrameCallbacks->onFrameStart = [this]()
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokeFrameStart();
            };

        m_pFrameCallbacks->onRender = [this]()
            {
                // ... todo ...
                // frustum cull via scene partition
                // build draw list form visible entities (transform index (gpu index) + mesh + material)
                // submit draw list to renderer
                // renderer binds transform SSBO and issues draw calls

                m_pUserFrameCallbacks->invokeRender();
            };

        m_pFrameCallbacks->onFrameEnd = [this]()
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokeFrameEnd();
            };

        m_pFrameCallbacks->onSyncPoint = [this](SystemGroup group, std::span<EntityChange const> entityChanges)
            {
                m_pSceneManager->processEntityChanges(*m_pWorld, entityChanges);
                m_pUserFrameCallbacks->invokeSyncPoint(group, entityChanges);
            };

        // ---------------------------------------------------------------------------------
        // Startup Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Startup)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Input Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Input)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Fixed Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::FixedUpdate)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Update)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Late Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::LateUpdate)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Pre-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PreRender)] = [this](SystemGroup group)
            {
                // ... todo ...
                // rebuild scene graph topological order
                // propagate world transforms -> write to mapped SSBO
                // update world bounds
                // update scene partition with new world positions / bounds

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Post-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PostRender)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Final Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Final)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };
    }

    std::shared_ptr<FrameCallbacks> EngineCallbacks::getFrameCallbacks() noexcept
    {
        return m_pFrameCallbacks;
    }
}