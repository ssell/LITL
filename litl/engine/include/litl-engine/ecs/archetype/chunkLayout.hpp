#ifndef LITL_ENGINE_ECS_ARCHETYPE_CHUNK_LAYOUT_H__
#define LITL_ENGINE_ECS_ARCHETYPE_CHUNK_LAYOUT_H__

#include <algorithm>
#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
{
    class Archetype;

    /// <summary>
    /// Describes the memory layout of all chunks within a single Archetype specialization.
    /// </summary>
    struct ChunkLayout
    {
        ChunkLayout()
            : archetype(nullptr), chunkElementCapacity(0), componentTypeCount(0)
        {
            componentOrder.fill(nullptr);
            componentOffsets.fill(0);
        }

        void reset() noexcept
        {
            archetype = nullptr;
            chunkElementCapacity = 0;
            componentTypeCount = 0;
            componentOrder.fill(nullptr);
            componentOffsets.fill(0);
        }

        /// <summary>
        /// Pointer back to the owning Archetype.
        /// </summary>
        Archetype* archetype;

        /// <summary>
        /// The number of entities and each component that can be stored within a Chunk.
        /// </summary>
        uint16_t chunkElementCapacity;

        /// <summary>
        /// The number of different component types stored in the chunk.
        /// </summary>
        uint8_t componentTypeCount;

        /// <summary>
        /// The order which the components appear within the chunk.
        /// The value is the component id.
        /// </summary>
        std::array<ComponentDescriptor*, MAX_COMPONENTS> componentOrder;

        /// <summary>
        /// The offset into the chunk that each component begins.
        /// </summary>
        std::array<uint16_t, MAX_COMPONENTS> componentOffsets;
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
    /// Fills out the ChunkLayout to fit the provided component types.
    /// </summary>
    /// <typeparam name="...ComponentTypes"></typeparam>
    /// <param name="layout"></param>
    template<typename... ComponentTypes>
    void populateChunkLayout(ChunkLayout* layout)
    {
        layout->reset();

        populateChunkLayoutComponents<ComponentTypes...>(layout, std::index_sequence_for<ComponentTypes...>{});

        // Sort components based on their ascending component type id. NULL components placed at the back.
        std::sort(layout->componentOrder.begin(), layout->componentOrder.end(), [](ComponentDescriptor* a, ComponentDescriptor* b)
            {
                return ((a == nullptr) ? false : (b == nullptr) ? true : a->id <= b->id);
            });
        
        for (auto i = 0; i < layout->componentOrder.size() && layout->componentOrder[i] != nullptr; ++i)
        {
            layout->componentTypeCount++;
        }

        // ... todo ... calculate capacity
        // ... todo ... calculate offsets
    }
}

#endif