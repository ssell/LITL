#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <vector>

#include "litl-core/containers/pagedVector.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/archetype/archetypeKey.hpp"
#include "litl-engine/ecs/archetype/archetypeColumn.hpp"
#include "litl-engine/ecs/archetype/chunkLayout.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// An Archetype is an ordered component set.
    /// All entities fit into exactly one Archetype.
    /// </summary>
    class Archetype
    {
    public:

        ChunkLayout const* layout() const noexcept;
        ArchetypeKey key() const noexcept;
        size_t entityCount() const noexcept;

        template<typename... ComponentTypes>
        static Archetype* build() noexcept
        {
            Archetype* archetype = new Archetype();
            populateChunkLayout<ComponentTypes...>(&archetype->m_chunkLayout);
            return archetype;
        }

    protected:

    private:

        Archetype();

        ChunkLayout m_chunkLayout;
        ArchetypeKey m_key;
        std::vector<Entity> m_entities;
        Core::PagedVector<Chunk> m_chunks;
    };
}

#endif