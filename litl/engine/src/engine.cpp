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

    // -------------------------------------------------------------------------------------
    // Engine
    // -------------------------------------------------------------------------------------

    Engine::Engine()
    {

    }

    Engine::~Engine()
    {

    }

    bool Engine::configureWindow(const char* title, uint32_t width, uint32_t height) const noexcept
    {
        //return cimpl()->pRenderer->initialize(title, width, height);
        return true;
    }
}