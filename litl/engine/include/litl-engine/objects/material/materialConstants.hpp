#ifndef LITL_ENGINE_OBJECTS_MATERIAL_CONSTANTS_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_CONSTANTS_H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// Number of individual slots per material property block.
    /// </summary>
    constexpr uint32_t MaterialSlotsPerBlock = 64u;

    /// <summary>
    /// Number of frames that a slot can be inactive before it is considered expired/unused.
    /// </summary>
    constexpr uint32_t MaterialSlotExpirationFrames = 8u;

    /// <summary>
    /// Numbers of frames in a row that a slot needs to be updated to be moved to the frequent update block.
    /// </summary>
    constexpr uint32_t MaterialSlotUpgradeToFrequentFrames = 8u;

    /// <summary>
    /// Number of frames that must elapse without a slot being updated before it is downgraded from the frequent update block.
    /// </summary>
    constexpr uint32_t MaterialSlotDowngradeFromFrequentFrames = 30u;

    /// <summary>
    /// Number of frames that a material bindings can be inactive before it is considered expired/unused.
    /// </summary>
    constexpr uint32_t MaterialBindingsExpirationFrames = MaterialSlotExpirationFrames;
}

#endif