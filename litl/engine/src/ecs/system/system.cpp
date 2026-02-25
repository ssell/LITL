#include "litl-engine/ecs/system/system.hpp"

namespace LITL::Engine::ECS
{
    namespace
    {
        static SystemTypeId nextSystemId()
        {
            static SystemTypeId id = 0;
            return id++;
        }
    }

    System::System()
        : id(nextSystemId())
    {

    }

    void System::run(World& world, float dt)
    {
        // todo get chunk and layout
        assert(m_runFunc != nullptr);
        //m_runFunc(world, dt, chunk, layout);
    }
}