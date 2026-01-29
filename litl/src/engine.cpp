#include "litl/engine/engine.hpp"
#include "litl/vulkan/renderer.hpp"

#include <new>

namespace LITL::Engine
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Engine::Impl
    {
        LITL::Vulkan::Renderer::Renderer renderer;
    };

    Engine::Impl* Engine::impl() noexcept
    {
        return std::launder(reinterpret_cast<Impl*>(m_storage));
    }

    Engine::Impl const* Engine::impl() const noexcept
    {
        return std::launder(reinterpret_cast<Impl const*>(m_storage));
    }

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine()
    {
        static_assert(sizeof(Impl) <= ImplSize, "Engine::Impl too large for Engine::m_storage");
        static_assert(alignof(Impl) <= ImplAlign, "Engine::Impl alignment too large.");

        new (&m_storage) Impl{};
    }

    Engine::~Engine()
    {
        impl()->~Impl();
    }

    bool Engine::configureWindow(const char* title, uint32_t width, uint32_t height) const noexcept
    {
        return impl()->renderer.initialize(title, width, height);
    }
}