#include "litl-core/logging/logging.hpp"
#include "litl-core/window.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/windowFactory.hpp"
#include "litl-engine/rendererFactory.hpp"

namespace LITL::Engine
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Engine::Impl
    {
        std::unique_ptr<LITL::Core::Window> pWindow;
        std::unique_ptr<LITL::Renderer::Renderer> pRenderer;

        Renderer::RendererDescriptor rendererDescriptor;
    };

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine(Renderer::RendererDescriptor const& rendererDescriptor)
    {
        LITL::Core::Logger::initialize("litl-engine", true, true);
        logInfo("LITL Engine Startup");

        m_impl->rendererDescriptor = rendererDescriptor;
    }

    Engine::~Engine()
    {
        logInfo("LITL Engine Shutdown");
        LITL::Core::Logger::shutdown();
    }

    bool Engine::openWindow(const char* title, uint32_t width, uint32_t height) noexcept
    {
        logInfo("Opening window \"", title, "\" (", width, "x", height, ") with ", Renderer::RendererBackendNames[m_impl->rendererDescriptor.rendererType], " backend ...");
        m_impl->pWindow = createWindow(m_impl->rendererDescriptor.rendererType);

        if (m_impl->pWindow == nullptr)
        {
            logCritical("Failed to create Window");
            return false;
        }

        if (!m_impl->pWindow->open(title, width, height))
        {
            logCritical("Failed to open Window");
            return false;
        }

        m_impl->pRenderer = createRenderer(m_impl->pWindow.get(), m_impl->rendererDescriptor);

        if (m_impl->pRenderer == nullptr)
        {
            logCritical("Failed to create Renderer");
            return false;
        }

        if (!m_impl->pRenderer->initialize())
        {
            logCritical("Failed to initialize Renderer");
            return false;
        }

        logInfo("... Window and Renderer creation successful.");
        return true;
    }

    bool Engine::shouldRun() noexcept
    {
        if (m_impl->pWindow == nullptr)
        {
            return false;
        }

        return !m_impl->pWindow->shouldClose();
    }
}