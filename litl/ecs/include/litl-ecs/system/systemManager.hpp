#ifndef LITL_ENGINE_SYSTEM_MANAGER_H__
#define LITL_ENGINE_SYSTEM_MANAGER_H__

#include <cstdint>
#include <memory>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemGroup.hpp"

namespace LITL::ECS
{
    class SystemManager
    {
    public:

        SystemManager();
        SystemManager(SystemManager const&) = delete;
        SystemManager& operator=(SystemManager const&) = delete;
        ~SystemManager();

        template<ValidSystem S>
        void addSystem(SystemGroup group) const noexcept
        {
            auto* system = getSystem<S>();
            system->template attach<S>();
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
        void updateSystemArchetypes() const noexcept;
        void runSchedule(SystemGroup group, World& world, float dt);

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

    };
}

#endif