#ifndef LITL_ENGINE_ECS_COMPONENTS_MESH_INSTANCE_H__
#define LITL_ENGINE_ECS_COMPONENTS_MESH_INSTANCE_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/assets/assetHandle.hpp"

namespace litl
{
    /// <summary>
    /// Represents a reference to a mesh asset instance that may not yet be in memory.
    /// Once the asset is in memory, then this component will be replaced by a MeshRef.
    /// Additionally, if the entity has a LocalBounds component then that will be automatically
    /// updated with the bounds of the mesh.
    /// 
    /// If the asset fails to load for any reason, this is instead replaced by a FailedMeshInstance.
    /// </summary>
    struct PendingMeshInstance
    {
        MeshAssetHandle meshHandle{};
    };

    /// <summary>
    /// Represents a mesh asset that failed to load.
    /// </summary>
    struct FailedMeshInstance
    {
        MeshAssetHandle meshHandle{};
    };
}

LITL_REGISTER_COMPONENT(litl::PendingMeshInstance);
LITL_REGISTER_COMPONENT(litl::FailedMeshInstance);

#endif