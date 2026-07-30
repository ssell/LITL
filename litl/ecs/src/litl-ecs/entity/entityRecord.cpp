#include "litl-ecs/entity/entityRecord.hpp"
#include "litl-ecs/archetype/archetype.hpp"

namespace litl
{
    void EntityRecord::update(Archetype& archetype, uint32_t index) noexcept
    {
        archetype = archetype;
        archetypeId = archetype.id();
        archetypeIndex = index;
    }
}