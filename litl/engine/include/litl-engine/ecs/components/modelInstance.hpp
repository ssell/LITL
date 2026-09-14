#ifndef LITL_ENGINE_ECS_COMPONENTS_MODEL_INSTANCE_H__
#define LITL_ENGINE_ECS_COMPONENTS_MODEL_INSTANCE_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/assets/assetHandle.hpp"

namespace litl
{
    /// <summary>
    /// Once the specified model is in memory, this will be replaced by the
    /// ModelInstance component and all accompanying components and child
    /// entities will in turn be created as well (MeshRef, MaterialRef, etc.).
    /// </summary>
    struct PendingModelInstance
    {
        ModelAssetHandle handle;
    };

    /// <summary>
    /// Indicates that the entity is the root of an instantiated model.
    /// 
    /// The user typically should not manually attach this component to an entity
    /// and should instead make use of PendingModelInstance.
    /// </summary>
    struct ModelInstance
    {
        ModelAssetHandle handle;
    };
}

LITL_REGISTER_COMPONENT(litl::PendingModelInstance);
LITL_REGISTER_COMPONENT(litl::ModelInstance);

#endif