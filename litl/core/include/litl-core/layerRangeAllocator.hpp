#ifndef LITL_CORE_LAYER_RANGE_ALLOCATOR_H__
#define LITL_CORE_LAYER_RANGE_ALLOCATOR_H__

#include <cstdint>
#include <optional>
#include <vector>

namespace litl
{
    struct LayerRange
    {
        uint32_t first{ 0u };
        uint32_t count{ 0u };

        [[nodiscard]] constexpr uint32_t end() const noexcept
        {
            return (first + count);
        }
    };

    /// <summary>
    /// Hands out contiguous ranges of array layers from a fixed [0, layerCount) span.
    /// Invariant: m_freeLayers is sorted by 'first', non-empty ranges only, and no two ranges touch or overlap (touching ranges are always merged).
    /// </summary>
    class LayerRangeAllocator
    {
    public:

        LayerRangeAllocator() noexcept = default;
        explicit LayerRangeAllocator(uint32_t layerCount) noexcept;

        /// <summary>
        /// Attempts to allocate a layer range of the specified size.
        /// Can fail if size is 0 or too large (allocator full or too fragmented).
        /// </summary>
        [[nodiscard]] std::optional<LayerRange> allocate(uint32_t count) noexcept;

        /// <summary>
        /// Frees the specified layer, releasing its range to be allocated in the future.
        /// </summary>
        void free(LayerRange range) noexcept;

        /// <summary>
        /// Number of free layers remaining. This is total across all layers, and is not guaranteed to be contiguous.
        /// </summary>
        [[nodiscard]] uint32_t freeLayers() const noexcept;

        /// <summary>
        /// Total capacity of the allocator. This includes used layers.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t capacity() const noexcept;

    private:

        uint32_t m_layerCount;
        std::vector<LayerRange> m_freeLayers;
    };
}

#endif