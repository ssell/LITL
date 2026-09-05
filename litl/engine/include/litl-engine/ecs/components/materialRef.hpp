#ifndef LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__
#define LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    /// <summary>
    /// Used to indicate that there is a single material associated with the entity.
    /// For entities/meshes that use multiple materials (submeshes) then use MultiMaterialRef.
    /// 
    /// If the mesh attached the entity has submeshes and this component is used, then the 
    /// same material will be applied to all submeshes.
    /// </summary>
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

    /// <summary>
    /// Used to indicate that there are multiple materials associated with the entity.
    /// If the mesh associated with the entity has more materials than those provided
    /// on this component, then the additional submeshes will use the first material.
    /// </summary>
    struct MultiMaterialRef
    {
        // ... todo ...
    };
}

LITL_REGISTER_COMPONENT(litl::MaterialRef);
LITL_REGISTER_COMPONENT(litl::MultiMaterialRef);

#endif