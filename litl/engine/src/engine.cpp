#include "litl-core/refPtr.hpp"
#include "litl-core/window.hpp"
#include "litl-core/logging/logging.hpp"
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
        std::unique_ptr<LITL::Core::Window> pWindow;
        std::unique_ptr<LITL::Renderer::Renderer> pRenderer;
        Core::RefPtr<LITL::Renderer::CommandBuffer> pFrameCommandBuffer;

        Configuration config;
        FrameLimiter frameLimiter;

        Core::JobScheduler jobScheduler;
        ECS::World ecsWorld;
    };

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine(Configuration const& config)
        : m_pImpl(std::make_unique<Engine::Impl>())
    {
        LITL::Core::Logger::initialize("litl-engine", true, true);
        logInfo("LITL Engine Startup");

        m_pImpl->config = config;
        m_pImpl->config.sanitize();
        m_pImpl->frameLimiter.setTargetFps(static_cast<float>(m_pImpl->config.simulationSettings.framesPerSecond));
    }

    Engine::~Engine()
    {
        logInfo("LITL Engine Shutdown");
        LITL::Core::Logger::shutdown();
    }

    bool Engine::openWindow(const char* title, uint32_t width, uint32_t height) noexcept
    {
        logInfo("Opening window \"", title, "\" (", width, "x", height, ") with ", Renderer::RendererBackendNames[m_pImpl->config.rendererSettings.rendererType], " backend ...");
        m_pImpl->pWindow = createWindow(m_pImpl->config.rendererSettings.rendererType);

        if (m_pImpl->pWindow == nullptr)
        {
            logCritical("Failed to create Window");
            return false;
        }

        if (!m_pImpl->pWindow->open(title, width, height))
        {
            logCritical("Failed to open Window");
            return false;
        }

        m_pImpl->pRenderer = createRenderer(m_pImpl->pWindow.get(), m_pImpl->config.rendererSettings);

        if (m_pImpl->pRenderer == nullptr)
        {
            logCritical("Failed to create Renderer");
            return false;
        }

        if (!m_pImpl->pRenderer->build())
        {
            logCritical("Failed to initialize Renderer");
            return false;
        }

        m_pImpl->pFrameCommandBuffer = m_pImpl->pRenderer->getResourceAllocator()->createCommandBuffer();

        logInfo("... Window and Renderer creation successful.");

        return true;
    }

    bool Engine::shouldRun() noexcept
    {
        if (m_pImpl->pWindow == nullptr)
        {
            return false;
        }

        return !m_pImpl->pWindow->shouldClose();
    }

    void Engine::run()
    {
        m_pImpl->frameLimiter.frameStart();

        update();
        render();

        m_pImpl->jobScheduler.wait();
        m_pImpl->frameLimiter.frameEnd();
    }

    void Engine::update()
    {
        m_pImpl->ecsWorld.run(
            m_pImpl->frameLimiter.frameDelta(), 
            1.0f / static_cast<float>(m_pImpl->config.simulationSettings.ticksPerSecond));
    }

    void Engine::render()
    {
        if (m_pImpl->pRenderer->beginRender())
        {
            m_pImpl->pRenderer->submitCommands(m_pImpl->pFrameCommandBuffer.get(), 0);
            m_pImpl->pRenderer->endRender();
        }
    }
}