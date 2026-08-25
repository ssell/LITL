#ifndef LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__
#define LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    struct MaterialRef
    {
        MaterialHandle handle;
        MaterialPropertySlotId slot;
    };
}

LITL_REGISTER_COMPONENT(litl::MaterialRef);

#endif