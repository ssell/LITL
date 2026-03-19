#include "litl-core/refPtr.hpp"
#include "litl-core/window.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/windowFactory.hpp"
#include "litl-engine/rendererFactory.hpp"
#include "litl-engine/frameLimiter.hpp"

#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace LITL::Engine
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Engine::Impl
    {
        std::unique_ptr<Core::ServiceCollection> pServiceCollection;
        std::shared_ptr<Core::ServiceProvider> pServiceProvider;

        // The below are also stored in pServiceProvider, but keep them in here to avoid having to frequently refetch.
        std::shared_ptr<Configuration> pSharedConfig;
        std::shared_ptr<FrameLimiter> pSharedFrameLimiter;
        std::shared_ptr<Core::Window> pSharedWindow;
        std::shared_ptr<Core::JobScheduler> pSharedJobScheduler;
        std::shared_ptr<ECS::World> pSharedECSWorld;
        std::shared_ptr<Renderer::Renderer> pSharedRenderer;
    };

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine()
        : m_pImpl(std::make_unique<Engine::Impl>())
    {
        LITL::Core::Logger::initialize("litl-engine", true, true);
        logInfo("LITL Engine Startup");

        m_pImpl->pServiceCollection->addSingleton<Configuration>();
        m_pImpl->pServiceCollection->addSingleton<FrameLimiter>();
        m_pImpl->pServiceCollection->addSingleton<Core::JobScheduler>();
        m_pImpl->pServiceCollection->addSingleton<ECS::World>();
        m_pImpl->pServiceCollection->addSingleton<Core::Window>();
        m_pImpl->pServiceCollection->addSingleton<Renderer::Renderer>();
        //m_pImpl->pServiceCollection->addSingleton<Core::RefPtr<Renderer::CommandBuffer>>();
    }

    Engine::~Engine()
    {
        logInfo("LITL Engine Shutdown");
        LITL::Core::Logger::shutdown();
    }

    void Engine::setup(Configuration config, ConfigureServicesFunc servicesFunc, ConfigureSystemsFunc systemsFunc) noexcept
    {
        if (servicesFunc != nullptr)
        {
            servicesFunc((*m_pImpl->pServiceCollection));
        }

        m_pImpl->pServiceProvider = m_pImpl->pServiceCollection->build();
        m_pImpl->pSharedConfig = m_pImpl->pServiceProvider->get<Configuration>();
        m_pImpl->pSharedFrameLimiter = m_pImpl->pServiceProvider->get<FrameLimiter>();
        m_pImpl->pSharedJobScheduler = m_pImpl->pServiceProvider->get<Core::JobScheduler>();
        m_pImpl->pSharedECSWorld = m_pImpl->pServiceProvider->get<ECS::World>();

        m_pImpl->pSharedConfig->set(config);
        m_pImpl->pSharedFrameLimiter->setTargetFps(static_cast<float>(m_pImpl->pSharedConfig->engineSettings.framesPerSecond));

        if (systemsFunc != nullptr)
        {
            systemsFunc((*m_pImpl->pSharedECSWorld));
        }
    }

    bool Engine::start()
    {
        if (!openWindow(
                m_pImpl->pSharedConfig->engineSettings.applicationName,
                m_pImpl->pSharedConfig->engineSettings.windowWidth,
                m_pImpl->pSharedConfig->engineSettings.windowHeight))
        {
            return false;
        }

        while (shouldRun())
        {
            run();
        }

        return true;
    }

    bool Engine::openWindow(const char* title, uint32_t width, uint32_t height) noexcept
    {
        logInfo("Opening window \"", title, "\" (", width, "x", height, ") with ", Renderer::RendererBackendNames[m_pImpl->pSharedConfig->rendererSettings.rendererType], " backend ...");
        
        auto window = createWindow(m_pImpl->pSharedConfig->rendererSettings.rendererType);

        if (window == nullptr)
        {
            logCritical("Failed to create window.");
            return false;
        }

        m_pImpl->pServiceProvider->setSingleton<Core::Window>(window);
        m_pImpl->pSharedWindow = m_pImpl->pServiceProvider->get<Core::Window>();

        if (!window->open(title, width, height))
        {
            logCritical("Failed to open Window");
            return false;
        }

        auto renderer = createRenderer(window, m_pImpl->pSharedConfig->rendererSettings);

        if (renderer == nullptr)
        {
            logCritical("Failed to create Renderer");
            return false;
        }

        m_pImpl->pServiceProvider->setSingleton<Renderer::Renderer>(renderer);
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
        return m_pImpl->pSharedWindow->shouldClose();
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