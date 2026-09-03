#include "litl-core/authority.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/scene/sceneManager.hpp"
#include "litl-engine/tasks/taskManager.hpp"
#include "litl-engine/objects/material/materialManager.hpp"

namespace litl
{
    struct EngineCallbacks::Impl final
    {
        std::shared_ptr<World> world{ nullptr };
        std::shared_ptr<AssetManager> assetManager{ nullptr };
        std::shared_ptr<RenderManager> renderManager{ nullptr };
        std::shared_ptr<SceneManager> sceneManager{ nullptr };
        std::shared_ptr<TaskManager> taskManager{ nullptr };
        std::shared_ptr<MaterialManager> materialManager{ nullptr };
        std::shared_ptr<FrameCallbacks> engineFrameCallbacks{ std::make_shared<FrameCallbacks>() };
        std::shared_ptr<FrameCallbacks> userFrameCallbacks{ nullptr };
    };

    EngineCallbacks::EngineCallbacks()
    {
        // ... needed as we use an ImplPtr which invokes this ...
    }

    EngineCallbacks::~EngineCallbacks()
    {
        // ... needed as we use an ImplPtr which invokes this ...
    }

    void EngineCallbacks::setup(Authority<Engine> authority, ServiceProvider& services, std::shared_ptr<FrameCallbacks> userCallbacks) noexcept
    {
        m_impl->world = services.get<World>();
        m_impl->assetManager = services.get<AssetManager>();
        m_impl->renderManager = services.get<RenderManager>();
        m_impl->sceneManager = services.get<SceneManager>();
        m_impl->taskManager = services.get<TaskManager>();
        m_impl->materialManager = services.get<MaterialManager>();
        m_impl->userFrameCallbacks = (userCallbacks != nullptr) ? userCallbacks : std::make_shared<FrameCallbacks>();

        LITL_FATAL_ASSERT_MSG((m_impl->world != nullptr), "Failed to inject World into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG((m_impl->assetManager != nullptr), "Failed to inject AssetManager into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG((m_impl->renderManager != nullptr), "Failed to inject RenderManager into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG((m_impl->sceneManager != nullptr), "Failed to inject SceneManager into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG((m_impl->sceneManager != nullptr), "Failed to inject TaskManager into EngineCallbacks");
        LITL_FATAL_ASSERT_MSG((m_impl->materialManager != nullptr), "Failed to inject MaterialManager into EngineCallbacks");

        // ---------------------------------------------------------------------------------
        // Intraframe Sync Points
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onSyncPoint = [this](ServiceProvider& services, SystemGroup group, std::span<EntityChange const> entityChanges)
            {
                m_impl->sceneManager->processEntityChanges({}, *m_impl->world, entityChanges);
                m_impl->userFrameCallbacks->invokeSyncPoint(services, group, entityChanges);
            };

        // ---------------------------------------------------------------------------------
        // Frame Start
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onFrameStart = [this](ServiceProvider& services, float dt)
            {
                m_impl->taskManager->update();
                m_impl->assetManager->onFrameStart();

                auto* renderer = m_impl->renderManager->getRenderer();

                if (renderer != nullptr)
                {
                    auto frameData = renderer->getFrameData();
                    m_impl->materialManager->onFrameStart({}, frameData.frameCount, frameData.frameInFlightIndex);
                }

                m_impl->userFrameCallbacks->invokeFrameStart(services, dt);
            };

        // ---------------------------------------------------------------------------------
        // Startup Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Startup)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Input Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Input)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Fixed Update Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::FixedUpdate)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Update Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Update)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Late Update Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::LateUpdate)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Pre-Render Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PreRender)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->sceneManager->onPreRender({}, * m_impl->world);
                m_impl->materialManager->onPreRender({});
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Render
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onRender = [this](ServiceProvider& services, float dt)
            {
                m_impl->renderManager->onRender({}, dt);
                m_impl->userFrameCallbacks->invokeRender(services, dt);
            };

        // ---------------------------------------------------------------------------------
        // Post-Render Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PostRender)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Final Group
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Final)] = [this](ServiceProvider& services, float dt, SystemGroup group)
            {
                m_impl->userFrameCallbacks->invokePreGroup(services, dt, group);
            };

        // ---------------------------------------------------------------------------------
        // Frame End
        // ---------------------------------------------------------------------------------

        m_impl->engineFrameCallbacks->onFrameEnd = [this](ServiceProvider& services, float dt)
            {
                m_impl->userFrameCallbacks->invokeFrameEnd(services, dt);
            };
    }

    std::shared_ptr<FrameCallbacks> EngineCallbacks::getFrameCallbacks() noexcept
    {
        return m_impl->engineFrameCallbacks;
    }
}