#include <new>
#include "litl-engine/engine.hpp"

namespace LITL::Engine
{
    // -------------------------------------------------------------------------------------
    // PIMPL
    // -------------------------------------------------------------------------------------

    struct Engine::Impl
    {
        uint8_t unused;
    };

    Engine::Impl* Engine::impl() noexcept
    {
        return std::launder(reinterpret_cast<Engine::Impl*>(&m_storage));
    }

    Engine::Impl const* Engine::cimpl() const noexcept
    {
        return std::launder(reinterpret_cast<Engine::Impl const*>(&m_storage));
    }

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine()
    {
        static_assert(sizeof(Engine::Impl) <= ImplSize, "Dedicated storage for Engine IMPL is too small");
        static_assert(alignof(Engine::Impl) <= ImplAlignment, "Alignment for Engine IMPL is too small");

        new (&m_storage) Engine::Impl{};
    }

    Engine::~Engine()
    {
        impl()->~Impl();
    }

    bool Engine::configureWindow(const char* title, uint32_t width, uint32_t height) const noexcept
    {
        //return cimpl()->pRenderer->initialize(title, width, height);
        return true;
    }
}