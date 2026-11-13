#include <algorithm>
#include <ranges>

#include "litl-core/assert.hpp"
#include "litl-core/layerRangeAllocator.hpp"

namespace litl
{
    LayerRangeAllocator::LayerRangeAllocator(uint32_t layerCount) noexcept
        : m_layerCount(layerCount)
    {
        if (layerCount > 0u)
        {
            m_freeLayers.push_back(LayerRange{ .first = 0u, .count = layerCount });
        }
    }

    std::optional<LayerRange> LayerRangeAllocator::allocate(uint32_t count) noexcept
    {
        if (count == 0u)
        {
            return std::nullopt;
        }

        // Find the lowest-addressed free range that is big enough to fit the specified count.
        const auto find = std::ranges::find_if(m_freeLayers, [count](LayerRange const& r) 
        { 
            return r.count >= count; 
        });

        if (find == m_freeLayers.end())
        {
            // Either full or too fragmented to fit the specified count.
            return std::nullopt;
        }

        const LayerRange result{ 
            .first = find->first, 
            .count = count
        };

        // Shrink the space in the free layer to account for the new allocation.
        find->first += count;
        find->count -= count;

        if (find->count == 0u)
        {
            // No room left, erase the layer.
            m_freeLayers.erase(find);
        }

        return result;
    }

    void LayerRangeAllocator::free(LayerRange range) noexcept
    {
        LITL_ASSERT((range.count != 0u) && (range.end() <= m_layerCount), );

        // First free range that starts after the one being returned.
        const auto next = std::ranges::upper_bound(m_freeLayers, range.first, std::less{}, &LayerRange::first);

        // Double-free / overlap detection against both neighbors.
        LITL_ASSERT((next == m_freeLayers.end()) || (range.end() <= next->first), );
        LITL_ASSERT((next == m_freeLayers.begin()) || (std::prev(next)->end() <= range.first), );

        const bool mergePrev = (next != m_freeLayers.begin()) && (std::prev(next)->end() == range.first);
        const bool mergeNext = (next != m_freeLayers.end()) && (range.end() == next->first);

        if (mergePrev && mergeNext)
        {
            auto prev = std::prev(next);
            prev->count += range.count + next->count;
            m_freeLayers.erase(next);
        }
        else if (mergePrev)
        {
            std::prev(next)->count += range.count;
        }
        else if (mergeNext)
        {
            next->first = range.first;
            next->count += range.count;
        }
        else
        {
            m_freeLayers.insert(next, range);
        }
    }

    uint32_t LayerRangeAllocator::freeLayers() const noexcept
    {
        uint32_t total = 0u;

        for (const auto& layer : m_freeLayers)
        {
            total += layer.count;
        }

        return total;
    }

    uint32_t LayerRangeAllocator::capacity() const noexcept
    {
        return m_layerCount;
    }
}