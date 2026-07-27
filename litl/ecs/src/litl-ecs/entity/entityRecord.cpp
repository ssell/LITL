#include "litl-ecs/entity/entityRecord.hpp"
#include "litl-ecs/archetype/archetype.hpp"

namespace litl
{
    void EntityRecord::update(Archetype* pArchetype, uint32_t index) noexcept
    {
        archetype = pArchetype;
        archetypeId = (archetype != nullptr ? archetype->id() : ecs::Constants::null_archetype_id);
        archetypeIndex = index;
    }
}