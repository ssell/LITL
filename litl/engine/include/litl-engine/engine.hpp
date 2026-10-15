#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include <memory>

#include "litl-engine/config.hpp"
#include "litl-engine/bootstrap.hpp"


namespace litl
{
    class Engine final
    {
    public:

        explicit Engine(EngineSetupFunctions setup = DefaultEngineSetup);
        ~Engine();

        Engine(Engine&&) = delete;
        Engine(Engine const&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(Engine const&) = delete;

        void setup(Configuration config, ConfigureServicesFunc servicesFunc, ConfigureSystemsFunc systemsFunc, ConfigureCallbacksFunc callbacksFunc, BootstrapFunc bootstrapFunc) noexcept;
        bool start() noexcept;
        void shutdown() noexcept;

    protected:

    private:

        bool createPlatform() noexcept;
        void run() noexcept;
        void update() noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif