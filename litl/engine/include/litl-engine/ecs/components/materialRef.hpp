#ifndef LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__
#define LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    struct MaterialRef
    {
        /// <summary>
        /// The versioned handle to the shared Material object.
        /// This is used in conjunction with the ObjectPool to retrieve a pointer to the Material.
        /// </summary>
        MaterialHandle handle;

        /// <summary>
        /// The slot that this instance occupies in the shared Material object.
        /// A slot is obtained by calling Material::allocateSlot.
        /// </summary>
        MaterialPropertySlotId slot;

        /// <summary>
        /// Indicates that the property data for this instance is stored in the frequent
        /// update block in the shared Material object. This should be set to true if
        /// the slot was either (A) obtained via Material::allocateSlot(true)
        /// or if (B) Material::markAsFrequentUpdate was called for the slot.
        /// 
        /// A material instance should be labelled as frequently updating if it is updating
        /// at a much faster rate than what is normal for other instances of the material.
        /// If it is normal for all instances to be updating rapidly then this should not be set.
        /// </summary>
        bool frequentUpdates{ false };
    };
}

LITL_REGISTER_COMPONENT(litl::MaterialRef);

#endif