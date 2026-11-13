#ifndef LITL_ENGINE_LIGHTS_SHADOW_ALLOCATION_H__
#define LITL_ENGINE_LIGHTS_SHADOW_ALLOCATION_H__

#include <cstdint>

#include "litl-core/constants.hpp"
#include "litl-core/layerRangeAllocator.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/lights/shadowCasterType.hpp"

namespace litl
{
    struct ShadowKey
    {
        Entity light{};
        uint32_t cameraSlot{ Constants::uint32_null_index };
        auto operator<=>(ShadowKey const&) const = default;
    };

    struct ShadowAllocation
    {
        /// <summary>
        /// The type of shadow caster that the allocation is for.
        /// </summary>
        ShadowCasterType type{ ShadowCasterType::Directional };

        /// <summary>
        /// Allocated range for this shadow caster.
        /// Different caster types allocate different counts of layers.
        /// For example: Directional = # of cascades, Spot = 1, Point = 6 (cubemap)
        /// </summary>
        LayerRange range{};

        /// <summary>
        /// Unique per allocation. Changes whenever the range changes.
        /// </summary>
        uint32_t version{ 0u };
    };

    /// <summary>
    /// Result of ShadowAllocator::acquire
    /// </summary>
    struct ShadowAcquireResult
    {
        ShadowAllocation allocation{};
        bool isNew{ false };
    };

    struct ShadowPoolCapacities
    {
        uint32_t directionalLayers{ 0u };
        uint32_t spotLayers{ 0u };
        uint32_t pointCubes{ 0u };
    };

    /// <summary>
    /// Maps (light,camera) keys to stable ranges within per-caster-type shadow pools.
    /// 
    /// Per-frame usage (main thread only):
    /// 
    ///     beginFrame(frame)                       <-- Recycles ranges that are safe to reuse.
    ///     acquire(key, type, count, frame)        <-- For every shadow wanted this frame.
    ///     releaseUnused(fame)                     <-- Retires every key not acquired this frame.
    /// 
    /// Ranges are never reused until framesInFlight frames after they were released,
    /// so in-flight GPU work never observes a range being handed to a new owner.
    /// </summary>
    class ShadowAllocator
    {
    public:

        /// <summary>
        /// Sets up the underlying layer allocators for each caster type.
        /// </summary>
        void setup(ShadowPoolCapacities const& capacities, uint32_t framesInFlight) noexcept;
        
        /// <summary>
        /// Recycles/frees allocated ranges that have been retired for framesInFlight frames.
        /// </summary>
        void beginFrame(uint32_t frame) noexcept;

        /// <summary>
        /// 
        /// </summary>
        [[nodiscard]] std::optional<ShadowAcquireResult> acquire(ShadowKey const& key, ShadowCasterType type, uint32_t count, uint32_t frame) noexcept;

        /// <summary>
        /// Retires all allocations that were not in used this frame.
        /// These allocations will then be freed and available for reused after framesInFlight frames.
        /// </summary>
        void releaseUnused(uint32_t frame) noexcept;

        /// <summary>
        /// Explicitly retires the specified allocation.
        /// This allocation will then be freed and available for reuse after framesInFlight frames.
        /// </summary>
        void release(ShadowKey const& key, uint32_t frame) noexcept;
        
        /// <summary>
        /// Returns the current number of active allocations.
        /// </summary>
        [[nodiscard]] uint32_t activeAllocationCount() const noexcept;

        /// <summary>
        /// Returns the number of retired allocations awaiting to be freed.
        /// </summary>
        [[nodiscard]] uint32_t pendingRetireCount() const noexcept;

        /// <summary>
        /// Returns the number of free/unused allocations for the specified caster type.
        /// </summary>
        [[nodiscard]] uint32_t freeUnits(ShadowCasterType type) const noexcept;

    private:

        struct Entry
        {
            ShadowKey key{};
            ShadowAllocation allocation{};
            uint32_t frameLastUsed{ 0u };
        };

        struct Retired
        {
            ShadowCasterType type{};
            LayerRange range{};
            uint32_t frameRetired{};
        };

        [[nodiscard]] LayerRangeAllocator& pool(ShadowCasterType type) noexcept;
        [[nodiscard]] LayerRangeAllocator const& pool(ShadowCasterType type) const noexcept;
        void retire(ShadowAllocation const& allocation, uint32_t frame) noexcept;

        std::array<LayerRangeAllocator, static_cast<uint32_t>(ShadowCasterType::Count)> m_pools{};
        std::vector<Entry> m_entries;
        std::vector<Retired> m_retired;
        uint32_t m_framesInFlight{ 2u };
        uint32_t m_versionCounter{ 0u };
    };
}

#endif