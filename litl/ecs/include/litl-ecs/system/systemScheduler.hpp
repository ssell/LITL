#ifndef LITL_ENGINE_SYSTEM_SCHEDULER_H__
#define LITL_ENGINE_SYSTEM_SCHEDULER_H__

#include <cstdint>
#include <memory>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemGroup.hpp"

namespace LITL::ECS
{
    class SystemScheduler
    {
    public:

        SystemScheduler();
        SystemScheduler(SystemScheduler const&) = delete;
        SystemScheduler& operator=(SystemScheduler const&) = delete;
        ~SystemScheduler();

        template<ValidSystem S>
        void addSystem(SystemGroup group) const noexcept
        {
            auto* system = getSystem<S>();
            system->attach<S>();
            addSystem(system, group);
        }

        void run(World& world);

    protected:

    private:

        template<ValidSystem S>
        static System* getSystem()
        {
            static System system;
            return &system;
        }

        void addSystem(System* system, SystemGroup group) const noexcept;
        void runSchedule(SystemGroup group, World& world, float dt);

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

    };
}

#endif