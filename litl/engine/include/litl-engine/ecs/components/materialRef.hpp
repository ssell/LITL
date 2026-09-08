#ifndef LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__
#define LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    /// <summary>
    /// Used to render a single material over an entire mesh and all of its submeshes.
    /// This is the standard material component to be used.
    /// </summary>
    struct MaterialRef
    {
        /// <summary>
        /// The versioned handle to the shared Material object.
        /// This is used in conjunction with the ObjectPool to retrieve a pointer to the Material.
        /// </summary>
        MaterialHandle handle{};

        /// <summary>
        /// The slot that this instance occupies in the shared Material object.
        /// A slot is obtained by calling Material::allocateSlot.
        /// </summary>
        MaterialPropertySlotId slot{};
    };

    /// <summary>
    /// Used to render a variable number of materials over a mesh, with each submesh being given its own assignment.
    /// </summary>
    struct VariableMaterialsRef
    {
        /// <summary>
        /// Handle to an engine-owned MaterialBindings instance that defines which materials are bound to which Mesh submeshes.
        /// </summary>
        MaterialBindingsHandle materialBindingsHandle;
    };
}

LITL_REGISTER_COMPONENT(litl::MaterialRef);
LITL_REGISTER_COMPONENT(litl::VariableMaterialsRef);

#endif