#ifndef LITL_ENGINE_SYSTEM_SCHEDULER_H__
#define LITL_ENGINE_SYSTEM_SCHEDULER_H__

#include <cstdint>
#include <memory>

#include "litl-engine/ecs/system/system.hpp"
#include "litl-engine/ecs/system/systemGroup.hpp"

namespace LITL::Engine::ECS
{
    class SystemScheduler
    {
    public:

        SystemScheduler();
        SystemScheduler(SystemScheduler const&) = delete;
        SystemScheduler& operator=(SystemScheduler const&) = delete;
        ~SystemScheduler();

        void addSystem(SystemTypeId systemId, SystemGroup group) const noexcept;

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

    };
}

#endif