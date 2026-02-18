#ifndef LITL_ENGINE_ECS_ARCHETYPE_CHUNK_LAYOUT_H__
#define LITL_ENGINE_ECS_ARCHETYPE_CHUNK_LAYOUT_H__

#include <algorithm>
#include <array>
#include <cstdint>
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
        ChunkLayout()
            : archetype(nullptr), chunkElementCapacity(0), componentTypeCount(0)
        {
            componentOrder.fill(nullptr);
            componentOffsets.fill(0);
        }

        void reset() noexcept
        {
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
        uint32_t chunkElementCapacity;

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
        std::array<ComponentDescriptor*, MAX_COMPONENTS> componentOrder;

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

        uint32_t componentBytesPerEntity = 0;
        
        for (auto i = 0; i < layout->componentOrder.size() && layout->componentOrder[i] != nullptr; ++i)
        {
            componentBytesPerEntity += layout->componentOrder[i]->size;
            layout->componentTypeCount++;
        }

        const uint32_t chunkHeaderSize = static_cast<uint32_t>(sizeof(ChunkHeader));
        const uint32_t chunkEntityArraySize = static_cast<uint32_t>(sizeof(ChunkEntities));
        uint32_t remaining = CHUNK_SIZE_BYTES - chunkHeaderSize - chunkEntityArraySize;

        // First estimate of how many entities can fit. This is close, but may not be exact due to alignment.
        layout->chunkElementCapacity = remaining / componentBytesPerEntity;

        // Get memory position of entity array
        uint32_t offset = chunkHeaderSize;
        offset = Math::alignMemoryOffsetUp(offset, alignof(Entity));
        layout->entityArrayOffset = static_cast<uint32_t>(offset);
        offset += chunkEntityArraySize;

        const uint32_t componentStartOffset = offset;
        uint32_t maxAttempts = 10; // loop guard

        // Calculate the starting offset of each aligned component column. May take a couple of reductions in the previous estimated capacity.
        while (maxAttempts-- > 0)
        {
            offset = componentStartOffset;

            for (size_t i = 0; i < layout->componentOrder.size() && layout->componentOrder[i] != nullptr; ++i)
            {
                // Get memory address for start of this component column
                offset = Math::alignMemoryOffsetUp(offset, layout->componentOrder[i]->alignment);
                layout->componentOffsets[i] = offset;

                // Move to the end of this column
                offset += layout->componentOrder[i]->size * layout->chunkElementCapacity;
            }

            if (offset <= CHUNK_SIZE_BYTES)
            {
                // All component columns fit in the chunk
                break;
            }

            // Remove one capacity to make room for alignment adjustments
            layout->chunkElementCapacity--;
        }
    }
}

#endif