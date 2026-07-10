#ifndef LITL_ENGINE_ECS_COMPONENTS_MESH_REF_H__
#define LITL_ENGINE_ECS_COMPONENTS_MESH_REF_H__

#include "litl-engine/objects/objectHandles.hpp"
#include "litl-ecs/register.hpp"

namespace litl
{
    struct MeshRef
    {
        MeshHandle handle;
    };
}

LITL_REGISTER_COMPONENT(litl::MeshRef);

#endif