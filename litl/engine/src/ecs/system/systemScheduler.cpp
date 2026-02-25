#include <array>
#include <mutex>
#include <vector>

#include "litl-engine/ecs/system/systemSchedule.hpp"
#include "litl-engine/ecs/system/systemScheduler.hpp"

namespace LITL::Engine::ECS
{
    namespace
    {
        
    }

    struct SystemScheduler::Impl
    {
        std::mutex systemsMutex;
        std::array<SystemSchedule, SystemGroupCount> schedules;
        std::vector<System*> systems;
    };

    SystemScheduler::SystemScheduler()
        : m_pImpl(std::make_unique<SystemScheduler::Impl>())
    {
        m_pImpl->schedules.fill({});
    }

    SystemScheduler::~SystemScheduler()
    {

    }

    void SystemScheduler::addSystem(System* system, SystemGroup group) const noexcept
    {
        {
            std::lock_guard<std::mutex> lock(m_pImpl->systemsMutex);

            if (m_pImpl->systems.size() < system->id)
            {
                m_pImpl->systems.resize(system->id * 2, nullptr);
            }
        }

        m_pImpl->systems[system->id] = system;
    }


    void SystemScheduler::run(World& world)
    {
        // todo calculate and pass dts
        float dt = 0.0f;
        float fixedDt = 0.0f;

        runSchedule(SystemGroup::Startup, world, dt);
        runSchedule(SystemGroup::Input, world, dt);

        // todo run multiple times potentially per frame
        runSchedule(SystemGroup::FixedUpdate, world, fixedDt);

        runSchedule(SystemGroup::Update, world, dt);
        runSchedule(SystemGroup::LateUpdate, world, dt);

        runSchedule(SystemGroup::PreRender, world, dt);
        runSchedule(SystemGroup::Render, world, dt);
        runSchedule(SystemGroup::PostRender, world, dt);

        runSchedule(SystemGroup::Final, world, dt);
    }

    void SystemScheduler::runSchedule(SystemGroup group, World& world, float dt)
    {
        while (m_pImpl->schedules[static_cast<uint32_t>(group)].run(world, dt, m_pImpl->systems))
        {
            // ... while the schedule has systems to run ...
            // ... todo insert escape mechanism ...
        }
    }
}