#include <string_view>
#include "litl-core/hash.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/component/componentRegistry.hpp"

namespace LITL::ECS
{
    ComponentTypeId ComponentDescriptor::nextId() noexcept
    {
        // Never assign id 0 to let it indicate an uninitialized component type id.
        static ComponentTypeId NextId = 1;
        return NextId++;
    }

    void ComponentDescriptor::track(ComponentDescriptor const* descriptor)
    {
        ComponentRegistry::track(descriptor);
    }

    StableComponentTypeId ComponentDescriptor::getStableId(std::string_view componentName) noexcept
    {
        return Core::hashString(componentName);
    }

    ComponentDescriptor const* ComponentDescriptor::get(ComponentTypeId componentTypeId) noexcept
    {
        return ComponentRegistry::find(componentTypeId);
    }
}