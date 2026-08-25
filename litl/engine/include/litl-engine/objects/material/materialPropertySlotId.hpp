#ifndef LITL_ENGINE_OBJECTS_MATERIAL_PROPERTY_SLOT_ID_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_PROPERTY_SLOT_ID_H__

#include <cstdint>
#include "litl-core/constants.hpp"

namespace litl
{
    struct MaterialPropertySlotId
    {
        /// <summary>
        /// The global slot index.
        /// </summary>
        uint32_t slot = Constants::uint32_null_index;

        /// <summary>
        /// The version/generation of the slot that this is valid for.
        /// </summary>
        uint32_t version = 0u;

        [[nodiscard]] constexpr bool isValid() const noexcept
        {
            return (slot != Constants::uint32_null_index) && (version != 0u);
        }
    };
}

#endif