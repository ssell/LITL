#ifndef LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__
#define LITL_ENGINE_ECS_COMPONENTS_MATERIAL_REF_H__

#include "litl-engine/objects/objectHandles.hpp"
#include "litl-ecs/register.hpp"

namespace litl
{
    struct MaterialRef
    {
        MaterialHandle handle;
    };
}

LITL_REGISTER_COMPONENT(litl::MaterialRef);

#endif