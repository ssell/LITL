#include "litl-core/window.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/windowFactory.hpp"

namespace LITL::Engine
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Engine::Impl
    {
        LITL::Renderer::RendererBackendType rendererType;
        LITL::Core::Window* pWindow;

        ~Impl()
        {
            if (pWindow != nullptr)
            {
                delete pWindow;
            }
        }
    };

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine(LITL::Renderer::RendererBackendType rendererType)
    {
        m_impl->rendererType = rendererType;
    }

    Engine::~Engine()
    {

    }

    bool Engine::openWindow(const char* title, uint32_t width, uint32_t height) noexcept
    {
        m_impl->pWindow = createWindow(m_impl->rendererType);

        if (m_impl->pWindow == nullptr)
        {
            return false;
        }

        return m_impl->pWindow->open(title, width, height);
    }
}