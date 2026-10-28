#include "litl-core/authority.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/scene/sceneManager.hpp"
#include "litl-engine/tasks/taskManager.hpp"
#include "litl-engine/objects/material/materialManager.hpp"
#include "litl-platform/platform.hpp"

namespace litl
{
    struct EngineCallbacks::Impl final
    {
        std::shared_ptr<World> world{ nullptr };
        std::shared_ptr<Platform> platform{ nullptr };
        std::shared_ptr<AssetManager> assetManager{ nullptr };
        std::shared_ptr<RenderManager> renderManager{ nullptr };
        std::shared_ptr<SceneManager> sceneManager{ nullptr };
        std::shared_ptr<TaskManager> taskManager{ nullptr };
        std::shared_ptr<MaterialManager> materialManager{ nullptr };
        std::shared_ptr<FrameCallbacks> engineFrameCallbacks{ std::make_shared<FrameCallbacks>() };
        std::shared_ptr<FrameCallbacks> userFrameCallbacks{ nullptr };
        std::shared_ptr<InputCallbacks> inputCallbacks{ std::make_shared<InputCallbacks>() };
        std::shared_ptr<WindowCallbacks> windowCallbacks{ std::make_shared<WindowCallbacks>() };

        void prepare(ServiceProvider& services) noexcept
        {
            world = services.get<World>();
            platform = services.get<Platform>();
            assetManager = services.get<AssetManager>();
            renderManager = services.get<RenderManager>();
            sceneManager = services.get<SceneManager>();
            taskManager = services.get<TaskManager>();
            materialManager = services.get<MaterialManager>();

            LITL_FATAL_ASSERT_MSG((world != nullptr), "Failed to inject World into EngineCallbacks");
            LITL_FATAL_ASSERT_MSG((platform != nullptr), "Failed to inject Platform into EngineCallbacks");
            LITL_FATAL_ASSERT_MSG((assetManager != nullptr), "Failed to inject AssetManager into EngineCallbacks");
            LITL_FATAL_ASSERT_MSG((renderManager != nullptr), "Failed to inject RenderManager into EngineCallbacks");
            LITL_FATAL_ASSERT_MSG((sceneManager != nullptr), "Failed to inject SceneManager into EngineCallbacks");
            LITL_FATAL_ASSERT_MSG((taskManager != nullptr), "Failed to inject TaskManager into EngineCallbacks");
            LITL_FATAL_ASSERT_MSG((materialManager != nullptr), "Failed to inject MaterialManager into EngineCallbacks");
        }

        void setup(ServiceProvider& services, std::shared_ptr<FrameCallbacks> userFrameCallbacks) noexcept
        {
            userFrameCallbacks = (userFrameCallbacks != nullptr) ? userFrameCallbacks : std::make_shared<FrameCallbacks>();

            // ---------------------------------------------------------------------------------
            // Intraframe Sync Points
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onSyncPoint = [this, userFrameCallbacks](ServiceProvider& services, SystemGroup group, std::span<EntityChange const> entityChanges)
                {
                    sceneManager->processEntityChanges({}, *world, entityChanges);
                    userFrameCallbacks->invokeSyncPoint(services, group, entityChanges);
                };

            // ---------------------------------------------------------------------------------
            // Frame Start
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onFrameStart = [this, userFrameCallbacks](ServiceProvider& services, float dt)
                {
                    auto* input = platform->getInput();

                    if (input != nullptr)
                    {
                        input->poll();
                        input->triggerCallbacks(services, *inputCallbacks);
                    }

                    auto* window = platform->getWindow();

                    if (window != nullptr)
                    {
                        window->triggerCallbacks(services, *windowCallbacks);
                    }

                    taskManager->update();
                    assetManager->onFrameStart();

                    auto* renderer = renderManager->getRenderer();

                    if (renderer != nullptr)
                    {
                        const auto frameData = renderer->getFrameData();
                        materialManager->onFrameStart({}, world->getFrame(), frameData.frameInFlightIndex);
                    }

                    userFrameCallbacks->invokeFrameStart(services, dt);
                };

            // ---------------------------------------------------------------------------------
            // Startup Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Startup)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Input Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Input)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Fixed Update Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::FixedUpdate)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Update Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Update)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Late Update Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::LateUpdate)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Pre-Render Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PreRender)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    sceneManager->onPreRender({}, *world);
                    materialManager->onPreRender({});
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Render
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onRender = [this, userFrameCallbacks](ServiceProvider& services, float dt)
                {
                    renderManager->onRender({}, dt);
                    userFrameCallbacks->invokeRender(services, dt);
                };

            // ---------------------------------------------------------------------------------
            // Post-Render Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::PostRender)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Final Group
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onPreGroup[static_cast<uint32_t>(SystemGroup::Final)] = [this, userFrameCallbacks](ServiceProvider& services, float dt, SystemGroup group)
                {
                    userFrameCallbacks->invokePreGroup(services, dt, group);
                };

            // ---------------------------------------------------------------------------------
            // Frame End
            // ---------------------------------------------------------------------------------

            engineFrameCallbacks->onFrameEnd = [this, userFrameCallbacks](ServiceProvider& services, float dt)
                {
                    userFrameCallbacks->invokeFrameEnd(services, dt);
                };

            // ---------------------------------------------------------------------------------
            // Input Events
            // ---------------------------------------------------------------------------------

            // ... none at the moment ...

            // ---------------------------------------------------------------------------------
            // Window Events
            // ---------------------------------------------------------------------------------

            // ... none at the moment ...
        }
    };

    EngineCallbacks::EngineCallbacks()
    {
        // ... needed as we use an ImplPtr which invokes this ...
    }

    EngineCallbacks::~EngineCallbacks()
    {
        // ... needed as we use an ImplPtr which invokes this ...
    }

    void EngineCallbacks::setup(Authority<Engine> authority, ServiceProvider& services, std::shared_ptr<FrameCallbacks> userFrameCallbacks) noexcept
    {
        m_impl->setup(services, userFrameCallbacks);
    }
    
    void EngineCallbacks::prepare(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_impl->prepare(services);
    }

    std::shared_ptr<FrameCallbacks> EngineCallbacks::getFrameCallbacks() noexcept
    {
        return m_impl->engineFrameCallbacks;
    }

    std::shared_ptr<InputCallbacks> EngineCallbacks::getInputCallbacks() noexcept
    {
        return m_impl->inputCallbacks;
    }

    std::shared_ptr<WindowCallbacks> EngineCallbacks::getWindowCallbacks() noexcept
    {
        return m_impl->windowCallbacks;
    }
}