#include <algorithm>
#include "litl-engine/lights/shadowAllocator.hpp"

namespace litl
{
    namespace
    {
        constexpr size_t index(ShadowCasterType type) noexcept
        {
            return static_cast<size_t>(type);
        }
    }


    void ShadowAllocator::setup(ShadowPoolCapacities const& capacities, uint32_t framesInFlight) noexcept
    {
        m_pools[index(ShadowCasterType::Directional)] = LayerRangeAllocator(capacities.directionalLayers);
        m_pools[index(ShadowCasterType::Spot)] = LayerRangeAllocator(capacities.spotLayers);
        m_pools[index(ShadowCasterType::Point)] = LayerRangeAllocator(capacities.pointCubes);

        m_framesInFlight = framesInFlight;
        m_entries.clear();
        m_retired.clear();
    }

    void ShadowAllocator::beginFrame(uint32_t frame) noexcept
    {
        const auto isSafeToRemove = [this, frame](Retired const& retired) { return (frame - retired.frameRetired) >= m_framesInFlight; };

        for (const auto& retired : m_retired)
        {
            if (isSafeToRemove(retired))
            {
                pool(retired.type).free(retired.range);
            }
        }

        std::erase_if(m_retired, isSafeToRemove);
    }

    std::optional<ShadowAcquireResult> ShadowAllocator::acquire(ShadowKey const& key, ShadowCasterType type, uint32_t count, uint32_t frame) noexcept
    {
        const auto find = std::ranges::lower_bound(m_entries, key, std::less{}, &Entry::key);

        if ((find != m_entries.end()) && (find->key == key))
        {
            // Same allocation type and range size. Mark as active and return.
            if ((find->allocation.type == type) && (find->allocation.range.count == count))
            {
                find->frameLastUsed = frame;
                return ShadowAcquireResult{ .allocation = find->allocation, .isNew = false };
            }
            // Same key but different shape. Retire and reallocate. Can be caused by change in cascade count, etc.
            else
            {
                retire(find->allocation, frame);
                m_entries.erase(find);
                return acquire(key, type, count, frame);
            }
        }
        else
        {
            const auto range = pool(type).allocate(count);

            if (!range.has_value())
            {
                // No room or too fragmented.
                return std::nullopt;
            }

            const ShadowAllocation allocation{
                .type = type,
                .range = *range,
                .version = ++m_versionCounter
            };

            m_entries.insert(find, Entry{
                .key = key,
                .allocation = allocation,
                .frameLastUsed = frame
            });

            return ShadowAcquireResult{
                .allocation = allocation,
                .isNew = true
            };
        }
    }

    void ShadowAllocator::releaseUnused(uint32_t frame) noexcept
    {
        for (const auto& entry : m_entries)
        {
            if (entry.frameLastUsed != frame)
            {
                retire(entry.allocation, frame);
            }
        }

        std::erase_if(m_entries, [frame](Entry const& entry)
            {
                return (entry.frameLastUsed != frame);
            });
    }

    void ShadowAllocator::release(ShadowKey const& key, uint32_t frame) noexcept
    {
        const auto find = std::ranges::lower_bound(m_entries, key, std::less{}, &Entry::key);

        if ((find != m_entries.end()) && (find->key == key))
        {
            retire(find->allocation, frame);
            m_entries.erase(find);
        }
    }

    uint32_t ShadowAllocator::activeAllocationCount() const noexcept
    {
        return static_cast<uint32_t>(m_entries.size());
    }

    uint32_t ShadowAllocator::pendingRetireCount() const noexcept
    {
        return static_cast<uint32_t>(m_retired.size());
    }

    uint32_t ShadowAllocator::freeUnits(ShadowCasterType type) const noexcept
    {
        return pool(type).freeLayers();
    }

    LayerRangeAllocator& ShadowAllocator::pool(ShadowCasterType type) noexcept
    {
        return m_pools[index(type)];
    }

    LayerRangeAllocator const& ShadowAllocator::pool(ShadowCasterType type) const noexcept
    {
        return m_pools[index(type)];
    }

    void ShadowAllocator::retire(ShadowAllocation const& allocation, uint32_t frame) noexcept
    {
        m_retired.push_back(Retired{
            .type = allocation.type,
            .range = allocation.range,
            .frameRetired = frame
        });
    }
}