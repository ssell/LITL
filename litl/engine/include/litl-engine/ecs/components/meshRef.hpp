#ifndef LITL_ENGINE_ECS_COMPONENTS_MESH_REF_H__
#define LITL_ENGINE_ECS_COMPONENTS_MESH_REF_H__

#include "litl-engine/objects/objectHandles.hpp"
#include "litl-ecs/register.hpp"

namespace litl
{
    /// <summary>
    /// A reference to an engine-owned Mesh object.
    /// This is one of the key components required to render an entity.
    /// 
    /// If the mesh is loaded from an asset, it is instead suggested  to use a PendingMeshInstance component 
    /// which will automatically be replaced by a valid MeshRef (and LocalBounds if present) on successful load.
    /// </summary>
    struct MeshRef
    {
        MeshHandle handle;
    };
}

LITL_REGISTER_COMPONENT(litl::MeshRef);

#endif