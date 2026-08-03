#ifndef LITL_ENGINE_CALLBACKS_H__
#define LITL_ENGINE_CALLBACKS_H__

#include <memory>

#include "litl-core/authority.hpp"
#include "litl-core/impl.hpp"
#include "litl-ecs/frameCallbacks.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;

    class EngineCallbacks
    {
    public:

        EngineCallbacks();
        ~EngineCallbacks();

        EngineCallbacks(EngineCallbacks const&) = delete;
        EngineCallbacks& operator=(EngineCallbacks const&) = delete;

        void setup(Authority<Engine> authority, ServiceProvider& services, std::shared_ptr<FrameCallbacks> userCallbacks) noexcept;
        std::shared_ptr<FrameCallbacks> getFrameCallbacks() noexcept;

    private:

        struct Impl;
        ImplPtr<Impl, 128u> m_impl;
    };
}

#endif