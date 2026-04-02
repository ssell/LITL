#include "litl-core/window.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/job/jobScheduler.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/windowFactory.hpp"
#include "litl-engine/rendererFactory.hpp"
#include "litl-engine/frameLimiter.hpp"


namespace LITL::Engine
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Engine::Impl
    {
        EngineSetupFunctions setup;

        Core::ServiceCollection serviceCollection;
        std::shared_ptr<Core::ServiceProvider> pServiceProvider{ nullptr };
        BootstrapFunc userBootstrap{ nullptr };

        // The below are also stored in pServiceProvider, but keep them in here to avoid having to frequently refetch.
        std::shared_ptr<Configuration> pSharedConfig{ nullptr };
        std::shared_ptr<FrameLimiter> pSharedFrameLimiter{ nullptr };
        std::shared_ptr<Core::Window> pSharedWindow{ nullptr };
        std::shared_ptr<Core::JobScheduler> pSharedJobScheduler{ nullptr };
        std::shared_ptr<ECS::World> pSharedECSWorld{ nullptr };
        std::shared_ptr<Renderer::Renderer> pSharedRenderer{ nullptr };
    };

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine(EngineSetupFunctions setup)
        : m_pImpl(std::make_unique<Engine::Impl>())
    {
        LITL::Core::Logger::initialize("litl-engine", true, true);
        logInfo("LITL Engine Startup");

        m_pImpl->setup = setup;
    }

    Engine::~Engine()
    {
        logInfo("LITL Engine Shutdown");
        LITL::Core::Logger::shutdown();
    }

    void Engine::setup(Configuration config, ConfigureServicesFunc servicesFunc, ConfigureSystemsFunc systemsFunc, BootstrapFunc bootstrapFunc) noexcept
    {
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
        m_pImpl->pSharedJobScheduler = m_pImpl->pServiceProvider->get<Core::JobScheduler>();
        m_pImpl->pSharedECSWorld = m_pImpl->pServiceProvider->get<ECS::World>();

        m_pImpl->pSharedConfig->set(config);
        m_pImpl->pSharedFrameLimiter->setTargetFps(static_cast<float>(m_pImpl->pSharedConfig->engineSettings.framesPerSecond));

        m_pImpl->setup.configureSystems(m_pImpl->pSharedECSWorld->getSystemCollection());

        if (systemsFunc != nullptr)
        {
            systemsFunc(m_pImpl->pSharedECSWorld->getSystemCollection());
        }

        m_pImpl->userBootstrap = bootstrapFunc;
    }

    bool Engine::start()
    {
        if (m_pImpl->pServiceProvider == nullptr)
        {
            logCritical("Attempted LITL Engine start prior to setup.");
            return false;
        }

        if (!createWindow() || !createRenderer())
        {
            return false;
        }

        m_pImpl->pSharedECSWorld->setup((*m_pImpl->pServiceProvider));

        m_pImpl->setup.bootstrap((*m_pImpl->pServiceProvider), (m_pImpl->pSharedECSWorld->getCommandBuffer()));

        if (m_pImpl->userBootstrap != nullptr)
        {
            logInfo("Bootstrapping ...");
            m_pImpl->userBootstrap((*m_pImpl->pServiceProvider), m_pImpl->pSharedECSWorld->getCommandBuffer());
        }

        logInfo("ECS system setup ...");

        m_pImpl->pSharedECSWorld->setupSystems((*m_pImpl->pServiceProvider));

        logInfo("Running ...");

        while (shouldRun())
        {
            run();
        }

        return true;
    }

    bool Engine::createWindow() noexcept
    {
        auto title = m_pImpl->pSharedConfig->engineSettings.applicationName;
        auto width = m_pImpl->pSharedConfig->engineSettings.windowWidth;
        auto height = m_pImpl->pSharedConfig->engineSettings.windowHeight;

        logInfo("Opening window \"", title, "\" (", width, "x", height, ") ...");
        
        if (!injectWindow((*m_pImpl->pServiceProvider), m_pImpl->pSharedConfig->rendererSettings.rendererType))
        {
            logCritical("Failed to create window.");
            return false;
        }

        m_pImpl->pSharedWindow = m_pImpl->pServiceProvider->get<Core::Window>();

        if (!m_pImpl->pSharedWindow->open(title, width, height))
        {
            logCritical("Failed to open Window");
            return false;
        }

        return true;
    }

    bool Engine::createRenderer() noexcept
    {
        logInfo("Creating renderer with ", Renderer::RendererBackendNames[m_pImpl->pSharedConfig->rendererSettings.rendererType], " backend ...");

        if (!injectRenderer((*m_pImpl->pServiceProvider), m_pImpl->pSharedWindow.get(), m_pImpl->pSharedConfig->rendererSettings))
        {
            logCritical("Failed to create Renderer");
            return false;
        }

        m_pImpl->pSharedRenderer = m_pImpl->pServiceProvider->get<Renderer::Renderer>();

        if (!m_pImpl->pSharedRenderer->build())
        {
            logCritical("Failed to initialize Renderer");
            return false;
        }

        //m_pImpl->pFrameCommandBuffer = m_pImpl->pSharedRenderer->getResourceAllocator()->createCommandBuffer();

        logInfo("... Window and Renderer creation successful.");

        return true;
    }

    bool Engine::shouldRun() noexcept
    {
        return !m_pImpl->pSharedWindow->shouldClose();
    }

    void Engine::run()
    {
        m_pImpl->pSharedFrameLimiter->frameStart();

        update();
        render();

        m_pImpl->pSharedJobScheduler->wait();
        m_pImpl->pSharedFrameLimiter->frameEnd();
    }

    void Engine::update()
    {
        m_pImpl->pSharedECSWorld->run(
            m_pImpl->pSharedFrameLimiter->frameDelta(),
            1.0f / static_cast<float>(m_pImpl->pSharedConfig->engineSettings.ticksPerSecond));
    }

    void Engine::render()
    {
        if (m_pImpl->pSharedRenderer->beginRender())
        {
            //m_pImpl->pSharedRenderer->submitCommands(m_pImpl->pFrameCommandBuffer.get(), 0);
            m_pImpl->pSharedRenderer->endRender();
        }
    }
}