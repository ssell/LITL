#include <vector>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/archetype/archetype.hpp"

namespace LITL::ECS
{
    struct System::Impl
    {
        const SystemTypeId id;
        // ... archetypes ...
    };

    namespace
    {
        static SystemTypeId nextSystemId()
        {
            static SystemTypeId id = 0;
            return id++;
        }
    }

    System::System()
        : m_pImpl(std::make_unique<System::Impl>(nextSystemId()))
    {

    }
    
    System::~System()
    {

    }

    SystemTypeId System::id() const noexcept
    {
        return m_pImpl->id;
    }

    void System::run(World& world, float dt)
    {
        // todo get chunk and layout
        assert(m_runFunc != nullptr);
        //m_runFunc(world, dt, chunk, layout);
    }
}