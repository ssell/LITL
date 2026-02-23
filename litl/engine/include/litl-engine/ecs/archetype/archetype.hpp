#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <span>
#include <string_view>
#include <vector>

#include "litl-core/debug.hpp"
#include "litl-core/containers/pagedVector.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entityRecord.hpp"
#include "litl-engine/ecs/archetype/chunkLayout.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"

namespace LITL::Engine::ECS
{
    class ArchetypeRegistry;
    class World;

    /// <summary>
    /// An Archetype is an ordered component set. All entities fit into exactly one Archetype.
    /// </summary>
    class Archetype : public DebugInfo
    {
    public:

        uint32_t registryIndex() const noexcept;
        uint64_t componentHash() const noexcept;
        ChunkLayout const* layout() const noexcept;
        std::vector<ComponentTypeId> const& componentTypes() const noexcept;
        uint32_t componentCount() const noexcept;

    protected:

    private:

        Archetype(std::string_view name, uint32_t registryIndex, uint64_t componentHash);

        uint32_t getNextIndex() noexcept;
        bool hasComponent(ComponentTypeId component, size_t& index);

        /// <summary>
        /// Adds a new entity to this archetype. Used for when the entity is first being created for it's version.
        /// This will construct all of the components.
        /// </summary>
        /// <param name="record"></param>
        void add(EntityRecord const& record) noexcept;

        /// <summary>
        /// Removes an entity from this archetype.
        /// </summary>
        /// <param name="record"></param>
        void remove(EntityRecord const& record) noexcept;

        /// <summary>
        /// Transfers the entity out of this archetype into the specified archetype.
        /// </summary>
        /// <param name="record"></param>
        /// <param name="to"></param>
        void move(EntityRecord const& record, Archetype* to) noexcept;

        const uint32_t m_registryIndex;
        const uint64_t m_componentHash;
        uint32_t m_entityCount;

        ChunkLayout m_chunkLayout;
        std::vector<ComponentTypeId> m_components;
        Core::PagedVector<Chunk> m_chunks;

        friend class ArchetypeRegistry;
        friend class World;
    };
}

#endif