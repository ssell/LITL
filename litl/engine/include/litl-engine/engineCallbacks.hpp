#ifndef LITL_ENGINE_CALLBACKS_H__
#define LITL_ENGINE_CALLBACKS_H__

#include <memory>

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;
    struct FrameCallbacks;
    class InputCallbacks;
    class WindowCallbacks;

    class EngineCallbacks
    {
    public:

        EngineCallbacks();
        ~EngineCallbacks();

        EngineCallbacks(EngineCallbacks const&) = delete;
        EngineCallbacks& operator=(EngineCallbacks const&) = delete;

        void setup(Authority<Engine> authority, ServiceProvider& services, std::shared_ptr<FrameCallbacks> userFrameCallbacks) noexcept;
        void prepare(Authority<Engine> authority, ServiceProvider& services) noexcept;
        std::shared_ptr<FrameCallbacks> getFrameCallbacks() noexcept;
        std::shared_ptr<InputCallbacks> getInputCallbacks() noexcept;
        std::shared_ptr<WindowCallbacks> getWindowCallbacks() noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 256> m_impl;
    };
}

#endif