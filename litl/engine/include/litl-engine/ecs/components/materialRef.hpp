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
    };
}

LITL_REGISTER_COMPONENT(litl::MaterialRef);

#endif