#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <span>
#include <string_view>
#include <vector>

#include "litl-core/debug.hpp"
#include "litl-core/containers/pagedVector.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/entityRecord.hpp"
#include "litl-ecs/archetype/chunkLayout.hpp"
#include "litl-ecs/archetype/chunk.hpp"

namespace LITL::ECS
{
    class ArchetypeRegistry;
    class World;

    /// <summary>
    /// An Archetype is an ordered component set. All entities fit into exactly one Archetype.
    /// </summary>
    class Archetype : public DebugInfo
    {
    public:

        ArchetypeId id() const noexcept;
        uint64_t componentHash() const noexcept;
        std::vector<ComponentTypeId> const& componentTypes() const noexcept;
        uint32_t componentCount() const noexcept;
        uint32_t entityCount() const noexcept;

        ChunkLayout const& chunkLayout() const noexcept;
        Chunk& getChunk(EntityRecord record) noexcept;
        Chunk& getChunk(uint32_t const index) noexcept;
        uint32_t chunkCount() const noexcept;

        template<ValidComponentType ComponentType>
        ComponentType& getComponent(EntityRecord record) noexcept
        {
            auto& chunk = getChunk(record);
            return chunk.getComponentArray<ComponentType>(m_chunkLayout)[record.archetypeIndex % m_chunkLayout.entityCapacity];
        }

        template<ValidComponentType ComponentType>
        bool hasComponent()
        {
            return hasComponent(ComponentDescriptor::get<ComponentType>()->id);
        }

        bool hasComponent(ComponentTypeId componentTypeId) const noexcept;
        bool hasComponent(ComponentTypeId componentTypeId, size_t& index) const noexcept;

        template<ValidComponentType ComponentType>
        void setComponent(EntityRecord record, ComponentType const& component) noexcept
        {
            auto& chunk = getChunk(record);
            chunk.getComponentArray<ComponentType>(m_chunkLayout)[record.archetypeIndex % m_chunkLayout.entityCapacity] = component;
        }

    protected:

    private:

        Archetype(std::string_view name, uint32_t id, uint64_t componentHash);

        uint32_t getNextIndex() noexcept;

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

        const ArchetypeId m_registryId;
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