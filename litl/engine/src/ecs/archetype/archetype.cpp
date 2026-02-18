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
        if (m_chunkLayout.componentTypeCount > 0)
        {
            m_key = Core::hash32(&m_chunkLayout.componentOrder[0]->id, sizeof(ComponentTypeId));

            for (size_t i = 1; i < m_chunkLayout.componentOrder.size() && m_chunkLayout.componentOrder[i] != nullptr; ++i)
            {
                uint32_t hashed = Core::hash32(&m_chunkLayout.componentOrder[i]->id, sizeof(ComponentTypeId));
                Core::hashCombine32(m_key, hashed);
            }
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