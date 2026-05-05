#include "litl-engine/engineCallbacks.hpp"

namespace litl
{
    void EngineCallbacks::setup(std::shared_ptr<ServiceProvider> serviceProvider, std::shared_ptr<FrameCallbacks> userCallbacks) noexcept
    {
        m_pSceneManager = serviceProvider->get<SceneManager>();
        m_pUserFrameCallbacks = (userCallbacks != nullptr) ? userCallbacks : std::make_shared<FrameCallbacks>();

        m_pFrameCallbacks->onFrameStart = [this]()
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokeFrameStart();
            };

        m_pFrameCallbacks->onFrameEnd = [this]()
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokeFrameEnd();
            };

        m_pFrameCallbacks->onSyncPoint = [this](SystemGroup group, std::span<EntityChange const> entityChanges)
            {
                // ... todo process scene commands ...

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

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::Startup)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Input Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Input)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::Input)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Fixed Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::FixedUpdate)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::FixedUpdate)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Update)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::Update)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Late Update Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::LateUpdate)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::LateUpdate)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Pre-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PreRender)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::PreRender)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Render)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::Render)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Post-Render Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PostRender)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::PostRender)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };

        // ---------------------------------------------------------------------------------
        // Final Group
        // ---------------------------------------------------------------------------------

        m_pFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Final)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePreGroup(group);
            };

        m_pFrameCallbacks->onPostGroup[static_cast<uint32_t>(SystemGroup::Final)] = [this](SystemGroup group)
            {
                // ... todo anything else? ...

                m_pUserFrameCallbacks->invokePostGroup(group);
            };
    }

    std::shared_ptr<FrameCallbacks> EngineCallbacks::getFrameCallbacks() noexcept
    {
        return m_pFrameCallbacks;
    }
}