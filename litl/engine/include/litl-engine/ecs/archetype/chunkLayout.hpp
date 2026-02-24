#ifndef LITL_ENGINE_ECS_ARCHETYPE_CHUNK_LAYOUT_H__
#define LITL_ENGINE_ECS_ARCHETYPE_CHUNK_LAYOUT_H__

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "litl-core/math/math.hpp"
#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/archetype/chunkHeader.hpp"

namespace LITL::Engine::ECS
{
    class Archetype;

    /// <summary>
    /// Describes the memory layout of all chunks within a single Archetype specialization.
    /// </summary>
    struct ChunkLayout
    {
        ChunkLayout();

        void calculate() noexcept;

        /// <summary>
        /// Retrieves the index (into componentOrder and componentOffsets) for the specified component type.
        /// Can throw if the specified component type is not present in this layout.
        /// </summary>
        /// <param name="componentTypeId"></param>
        /// <param name="index"></param>
        void getComponentIndex(ComponentTypeId componentTypeId, uint32_t& index) const;

        /// <summary>
        /// Pointer back to the owning Archetype.
        /// </summary>
        Archetype* archetype;

        /// <summary>
        /// The number of entities and each component that can be stored within a Chunk.
        /// </summary>
        uint32_t entityCapacity;

        /// <summary>
        /// The number of different component types stored in the chunk.
        /// </summary>
        uint32_t componentTypeCount;

        /// <summary>
        /// The offset into the chunk that the entity array begins.
        /// </summary>
        uint32_t entityArrayOffset;

        /// <summary>
        /// The order which the components appear within the chunk.
        /// The value is the component id.
        /// </summary>
        std::array<ComponentDescriptor const*, MAX_COMPONENTS> componentOrder;

        /// <summary>
        /// The offset into the chunk that each component begins.
        /// </summary>
        std::array<uint32_t, MAX_COMPONENTS> componentOffsets;
    };

    /// <summary>
    /// Inserts the ComponentDescriptor into the layout.
    /// </summary>
    /// <typeparam name="ComponentType"></typeparam>
    /// <typeparam name="Index"></typeparam>
    /// <param name="layout"></param>
    template<typename ComponentType, std::size_t Index>
    void populateChunkLayoutComponent(ChunkLayout* layout)
    {
        layout->componentOrder[Index] = ComponentDescriptor::get<ComponentType>();
    }

    /// <summary>
    /// Iterates over all of the provided component types.
    /// </summary>
    /// <typeparam name="...ComponentTypes"></typeparam>
    /// <typeparam name="...Index"></typeparam>
    /// <param name="layout"></param>
    /// <param name=""></param>
    template<typename... ComponentTypes, size_t... Index>
    void populateChunkLayoutComponents(ChunkLayout* layout, std::index_sequence<Index...>)
    {
        (populateChunkLayoutComponent<ComponentTypes, Index>(layout), ...);
    }

    /// <summary>
    /// Fills out the ChunkLayout from the provided runtime list of type ids.
    /// </summary>
    /// <param name="layout"></param>
    /// <param name="orderedComponentTypes"></param>
    void populateChunkLayout(ChunkLayout* layout, std::span<ComponentTypeId const> orderedComponentTypes);

    /// <summary>
    /// Fills out the ChunkLayout to fit the provided component types.
    /// </summary>
    /// <typeparam name="...ComponentTypes"></typeparam>
    /// <param name="layout"></param>
    template<typename... ComponentTypes>
    void populateChunkLayout(ChunkLayout* layout)
    {
        populateChunkLayoutComponents<ComponentTypes...>(layout, std::index_sequence_for<ComponentTypes...>{});
        layout->calculate();
    }
}

#endif