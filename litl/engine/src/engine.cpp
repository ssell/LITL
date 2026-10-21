#include "litl-core/thread.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/job/jobScheduler.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-ecs/frameCallbacks.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/platformFactory.hpp"
#include "litl-platform/platform.hpp"
#include "litl-platform/window.hpp"
#include "litl-engine/frameLimiter.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/materialManager.hpp"
#include "litl-engine/scene/sceneManager.hpp"
#include "litl-engine/tasks/taskManager.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/input/inputActions.hpp"

namespace litl
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    namespace
    {
        enum class EngineState : uint32_t
        {
            Waiting = 0u,
            Ready = 1u,
            Starting = 2u,
            Running = 3u,
            Stopped = 4u,
            Shutdown = 5u,
            Failed = 6u
        };
    }

    struct Engine::Impl
    {
        EngineSetupFunctions setup;

        ServiceCollection serviceCollection;
        std::shared_ptr<ServiceProvider> pServiceProvider{ nullptr };
        BootstrapFunc userBootstrap{ nullptr };
        EngineCallbacks callbacks;
        EngineState state{ EngineState::Waiting };

        // The below are also stored in pServiceProvider, but keep them in here to avoid having to frequently refetch.
        std::shared_ptr<Configuration> pSharedConfig{ nullptr };
        std::shared_ptr<FrameLimiter> pSharedFrameLimiter{ nullptr };
        std::shared_ptr<Platform> pSharedPlatform{ nullptr };
        std::shared_ptr<Window> pSharedWindow{ nullptr };
        std::shared_ptr<Input> pSharedInput{ nullptr };
        std::shared_ptr<InputActions> pSharedInputActions{ nullptr };
        std::shared_ptr<JobScheduler> pSharedJobScheduler{ nullptr };
        std::shared_ptr<World> pSharedECSWorld{ nullptr };
        std::shared_ptr<RenderManager> pSharedRenderManager{ nullptr };
        std::shared_ptr<TaskManager> pSharedTaskManager{ nullptr };
        std::shared_ptr<ObjectPool> pSharedObjectPool{ nullptr };
        std::shared_ptr<MaterialManager> pSharedMaterialManager{ nullptr };
        std::shared_ptr<AssetManager> pSharedAssetManager{ nullptr };
        std::shared_ptr<SceneManager> pSharedSceneManager{ nullptr };
        
        void configureCallbacks(ConfigureFrameCallbacksFunc userFrameCallbacksFunc, ConfigureInputCallbacksFunc userInputCallbacksFunc)
        {
            std::shared_ptr<FrameCallbacks> userFrameCallbacks = std::make_shared<FrameCallbacks>();

            if (userFrameCallbacksFunc != nullptr)
            {
                userFrameCallbacksFunc(userFrameCallbacks);
            }

            if (userInputCallbacksFunc != nullptr)
            {
                userInputCallbacksFunc(callbacks.getInputCallbacks());          // Engine and User share the same InputCallbacks instance so that users can set higher priority items if needed.
            }

            callbacks.setup({}, *pServiceProvider, userFrameCallbacks);
        }

        void prepareCallbacks()
        {
            callbacks.prepare({}, *pServiceProvider);
        }
    };

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine(EngineSetupFunctions setup)
        : m_pImpl(std::make_unique<Engine::Impl>())
    {
        litl::Logger::initialize("litl-engine", true, true);
        logInfo("LITL Engine Startup");

        m_pImpl->setup = setup;
        ThreadInfo::setMainThread();
    }

    Engine::~Engine()
    {
        shutdown();
    }

    void Engine::setup(
        Configuration config, 
        ConfigureServicesFunc servicesFunc, 
        ConfigureSystemsFunc systemsFunc, 
        ConfigureFrameCallbacksFunc frameCallbacksFunc,
        ConfigureInputCallbacksFunc inputCallbacksFunc,
        BootstrapFunc bootstrapFunc) noexcept
    {
        if (m_pImpl->state != EngineState::Waiting)
        {
            return;
        }

        m_pImpl->state = EngineState::Ready;

        // These can be modified by the user (though it is unusual), so make sure they exist first.
        assert(m_pImpl->setup.configureServices != nullptr);
        assert(m_pImpl->setup.configureSystems != nullptr);
        assert(m_pImpl->setup.bootstrap != nullptr);

        m_pImpl->setup.configureServices(m_pImpl->serviceCollection);

        if (servicesFunc != nullptr)
        {
            servicesFunc(m_pImpl->serviceCollection);
        }

        m_pImpl->pServiceProvider = m_pImpl->serviceCollection.build();
        m_pImpl->pSharedConfig = m_pImpl->pServiceProvider->get<Configuration>();
        m_pImpl->pSharedFrameLimiter = m_pImpl->pServiceProvider->get<FrameLimiter>();
        m_pImpl->pSharedJobScheduler = m_pImpl->pServiceProvider->get<JobScheduler>();
        m_pImpl->pSharedECSWorld = m_pImpl->pServiceProvider->get<World>();
        m_pImpl->pSharedTaskManager = m_pImpl->pServiceProvider->get<TaskManager>();
        m_pImpl->pSharedObjectPool = m_pImpl->pServiceProvider->get<ObjectPool>();
        m_pImpl->pSharedMaterialManager = m_pImpl->pServiceProvider->get<MaterialManager>();
        m_pImpl->pSharedAssetManager = m_pImpl->pServiceProvider->get<AssetManager>();
        m_pImpl->pSharedSceneManager = m_pImpl->pServiceProvider->get<SceneManager>();
        m_pImpl->pSharedRenderManager = m_pImpl->pServiceProvider->get<RenderManager>();

        m_pImpl->pSharedTaskManager->setup({}, *m_pImpl->pServiceProvider);
        m_pImpl->pSharedObjectPool->setup({}, *m_pImpl->pServiceProvider);
        m_pImpl->pSharedMaterialManager->setup({}, *m_pImpl->pServiceProvider);
        m_pImpl->pSharedAssetManager->setup({}, *m_pImpl->pServiceProvider);
        m_pImpl->pSharedSceneManager->setup({}, (*m_pImpl->pServiceProvider));
        m_pImpl->pSharedConfig->set(config);
        m_pImpl->pSharedFrameLimiter->setTargetFps(static_cast<float>(m_pImpl->pSharedConfig->engine.framesPerSecond));

        m_pImpl->setup.configureSystems(m_pImpl->pSharedECSWorld->getSystemCollection());

        if (systemsFunc != nullptr)
        {
            systemsFunc(m_pImpl->pSharedECSWorld->getSystemCollection());
        }

        m_pImpl->userBootstrap = bootstrapFunc;
        m_pImpl->configureCallbacks(frameCallbacksFunc, inputCallbacksFunc);
    }

    void Engine::shutdown() noexcept
    {
        if (m_pImpl->state == EngineState::Shutdown)
        {
            return;
        }

        logInfo("LITL Engine Shutdown ...");
        m_pImpl->state = EngineState::Shutdown;

        if (m_pImpl->pSharedRenderManager != nullptr)
        {
            logInfo("Halting Renderer ...");
            m_pImpl->pSharedRenderManager->halt();
        }

        if (m_pImpl->pSharedAssetManager != nullptr)
        {
            logInfo("Shutdown Asset Manager ...");
            m_pImpl->pSharedAssetManager->destroy({});
        }

        if (m_pImpl->pSharedObjectPool != nullptr)
        {
            logInfo("Shutdown Object Pool ...");
            m_pImpl->pSharedObjectPool->destroy({});
        }

        if (m_pImpl->pSharedRenderManager != nullptr)
        {
            logInfo("Shutdown Render Manager ...");
            m_pImpl->pSharedRenderManager->destroy({});
        }

        if (m_pImpl->pSharedPlatform != nullptr)
        {
            logInfo("Shutdown Platform ...");

            if (!m_pImpl->pSharedPlatform->shutdown())
            {
                logInfo("... Platform shutdown failed.");
            }
        }

        logInfo("Shutdown Logger. Goodbye.");
        litl::Logger::shutdown();
    }

    bool Engine::start() noexcept
    {
        if (m_pImpl->state != EngineState::Ready)
        {
            logWarning("Attempting to start engine that is not in Ready state.");
            return false;
        }

        if (m_pImpl->pServiceProvider == nullptr)
        {
            logCritical("Attempted LITL Engine start prior to setup.");
            m_pImpl->state = EngineState::Failed;
            return false;
        }

        // Setup Platform
        if (!createPlatform())
        {
            m_pImpl->state = EngineState::Failed;
            return false;
        }

        // Setup Renderer
        m_pImpl->pSharedRenderManager->setup({}, (*m_pImpl->pServiceProvider));

        // Setup ECS
        m_pImpl->pSharedECSWorld->setup((*m_pImpl->pServiceProvider), m_pImpl->callbacks.getFrameCallbacks());
        m_pImpl->setup.bootstrap((*m_pImpl->pServiceProvider), (m_pImpl->pSharedECSWorld->getCommandBuffer()));

        // Setup Scene
        m_pImpl->pSharedSceneManager->createScene(m_pImpl->pSharedConfig->scene);

        m_pImpl->prepareCallbacks();

        if (m_pImpl->userBootstrap != nullptr)
        {
            logInfo("Bootstrapping ...");
            m_pImpl->userBootstrap((*m_pImpl->pServiceProvider), m_pImpl->pSharedECSWorld->getCommandBuffer());
        }

        logInfo("ECS system setup ...");

        m_pImpl->pSharedECSWorld->finalize();

        logInfo("Running ...");
        m_pImpl->state = EngineState::Running;

        while (true)
        {
            m_pImpl->pSharedWindow->pollForEvents();

            if (m_pImpl->pSharedWindow->closeRequested())
            {
                logInfo("Window closing.");
                break;
            }

            run();
        }

        m_pImpl->state = EngineState::Stopped;
        return true;
    }

    bool Engine::createPlatform() noexcept
    {
        // ---------------------------------------------------------------------------------
        // Platform
        // ---------------------------------------------------------------------------------

        logInfo("Creating platform ...");
        
        if (!injectPlatform((*m_pImpl->pServiceProvider), m_pImpl->pSharedConfig->platform.platformType))
        {
            return false;
        }

        m_pImpl->pSharedPlatform = m_pImpl->pServiceProvider->get<Platform>();

        if (m_pImpl->pSharedPlatform == nullptr)
        {
            logCritical("Failed to retrieve injected platform.");
            return false;
        }

        // ---------------------------------------------------------------------------------
        // Input
        // ---------------------------------------------------------------------------------

        logInfo("Creating input layer ...");

        auto* input = m_pImpl->pSharedPlatform->createInput();

        if (input == nullptr)
        {
            logCritical("Failed to create input layer.");
            return false;
        }

        if (!m_pImpl->pServiceProvider->setSingleton<Input, Input>(input, false))
        {
            logCritical("Failed to inject input layer.");
            return false;
        }

        m_pImpl->pSharedInput = m_pImpl->pServiceProvider->get<Input>();
        m_pImpl->pSharedInputActions = m_pImpl->pServiceProvider->get<InputActions>();
        m_pImpl->pSharedInputActions->setup({}, *m_pImpl->pServiceProvider);

        // ---------------------------------------------------------------------------------
        // Window
        // ---------------------------------------------------------------------------------

        auto title = m_pImpl->pSharedConfig->platform.title;
        auto width = m_pImpl->pSharedConfig->platform.windowWidth;
        auto height = m_pImpl->pSharedConfig->platform.windowHeight;

        logInfo("Opening window \"", title, "\" (", width, "x", height, ") ...");

        auto* window = m_pImpl->pSharedPlatform->createWindow();

        if (window == nullptr)
        {
            logCritical("Failed to create window.");
            return false;
        }

        if (!m_pImpl->pServiceProvider->setSingleton<Window, Window>(window, false))
        {
            logCritical("Failed to inject window.");
            return false;
        }

        if (!window->open(title, width, height))
        {
            logCritical("Failed to open window.");
            return false;
        }

        m_pImpl->pSharedWindow = m_pImpl->pServiceProvider->get<Window>();

        return true;
    }

    void Engine::run() noexcept
    {
        m_pImpl->pSharedFrameLimiter->frameStart();

        update();

        m_pImpl->pSharedJobScheduler->wait();
        m_pImpl->pSharedFrameLimiter->frameEnd();
    }

    void Engine::update() noexcept
    {
        m_pImpl->pSharedECSWorld->run(
            m_pImpl->pSharedFrameLimiter->frameDelta(),
            1.0f / static_cast<float>(m_pImpl->pSharedConfig->engine.ticksPerSecond));
    }
}