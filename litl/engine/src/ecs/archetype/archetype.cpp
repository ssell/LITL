#include <algorithm>
#include <vector>

#include "litl-core/hash.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"

namespace LITL::Engine::ECS
{
    Archetype::Archetype()
        : m_key(0), m_chunks(16)      // 16kb chunks * 16 = 256kb pages
    {
        m_chunkLayout.archetype = this;
    }

    void Archetype::buildArchetypeKey() noexcept
    {
        std::vector<ComponentTypeId> archetypeComponentTypeIds;
        archetypeComponentTypeIds.reserve(m_chunkLayout.componentTypeCount);

        if (m_chunkLayout.componentTypeCount > 0)
        {
            for (uint32_t i = 0; i < m_chunkLayout.componentTypeCount; ++i)
            {
                archetypeComponentTypeIds.push_back(m_chunkLayout.componentOrder[i]->id);
            }

            m_key = Core::hashArray<ComponentTypeId>(archetypeComponentTypeIds);
        }
    }

    uint32_t Archetype::key() const noexcept
    {
        return m_key;
    }

    ChunkLayout const* Archetype::layout() const noexcept
    {
        return &m_chunkLayout;
    }

    size_t Archetype::entityCount() const noexcept
    {
        return m_entities.size();
    }
}