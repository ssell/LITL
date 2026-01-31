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
        LITL::Core::Window* pWindow;

        Renderer::RendererDescriptor rendererDescriptor;
        LITL::Renderer::Renderer* pRenderer;

        ~Impl()
        {
            if (pRenderer != nullptr)
            {
                delete pRenderer;
            }

            if (pWindow != nullptr)
            {
                delete pWindow;
            }
        }
    };

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine(Renderer::RendererDescriptor const& rendererDescriptor)
    {
        m_impl->rendererDescriptor = rendererDescriptor;
    }

    Engine::~Engine()
    {

    }

    bool Engine::openWindow(const char* title, uint32_t width, uint32_t height) noexcept
    {
        m_impl->pWindow = createWindow(m_impl->rendererDescriptor.rendererType);

        if (m_impl->pWindow == nullptr)
        {
            // todo log error
            return false;
        }

        if (!m_impl->pWindow->open(title, width, height))
        {
            // todo log error
            return false;
        }

        m_impl->pRenderer = createRenderer(m_impl->rendererDescriptor);

        if (m_impl->pRenderer == nullptr)
        {
            // todo log error
            return false;
        }

        if (!m_impl->pRenderer->initialize())
        {
            // todo log error
            return false;
        }

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