#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
{
    ComponentTypeId ComponentDescriptor::nextId() noexcept
    {
        // Never assign id 0 to let it indicate an uninitialized component type id.
        static ComponentTypeId NextId = 1;
        return NextId++;
    }

    ComponentDescriptor const* ComponentDescriptor::get(ComponentTypeId componentTypeId) noexcept
    {
        // ... next todo ... 
        return nullptr;
    }
}