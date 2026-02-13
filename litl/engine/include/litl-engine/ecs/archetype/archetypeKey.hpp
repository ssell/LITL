#ifndef LITL_ENGINE_ECS_ARCHETYPE_KEY_H__
#define LITL_ENGINE_ECS_ARCHETYPE_KEY_H__

#include <cstdint>
#include <vector>

#include "litl-core/hash.hpp"
#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
{
    struct ArchetypeKey
    {
        std::vector<ComponentTypeId> componentTypes;

        bool operator==(ArchetypeKey const& other) const
        {
            return componentTypes == other.componentTypes;
        }
    };

    struct ArchetypeKeyHasher
    {
        size_t operator()(ArchetypeKey const& key) const noexcept
        {
            return static_cast<size_t>(Core::hashArray<ComponentTypeId>(key.componentTypes));
        }
    };

}

#endif