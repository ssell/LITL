#ifndef LITL_ENGINE_ECS_COMPONENTS_MODEL_INSTANCE_H__
#define LITL_ENGINE_ECS_COMPONENTS_MODEL_INSTANCE_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/assets/assetHandle.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    /// <summary>
    /// Once the specified model is in memory, this will be replaced by the
    /// ModelInstance component and all accompanying components and child entities will in turn be created as well (MeshRef, MaterialRef, etc.).
    /// If the model fails to load then this component is replaced with a FailedModelInstance component.
    /// </summary>
    struct PendingModelInstance
    {
        /// <summary>
        /// The handle to the Model asset that this component is waiting to load.
        /// Once this model is in memory, then all of its meshes will be added as individual children to the entity.
        /// </summary>
        ModelAssetHandle modelHandle;

        /// <summary>
        /// In the event a mesh does not have a material assigned to it, or if its material failed to load, then
        /// this is the default material applied to the mesh as a fallback.
        /// </summary>
        MaterialHandle fallbackMaterialHandle{};

        /// <summary>
        /// In the event the fallback material is used, then this is the slot associated with the model instance.
        /// If no slot is provided then one will be allocated if/when necessary.
        /// </summary>
        MaterialPropertySlotId fallbackMaterialSlot{};
    };

    /// <summary>
    /// Indicates that the entity is the root of an instantiated model.
    /// 
    /// The user typically should not manually attach this component to an entity
    /// and should instead make use of PendingModelInstance.
    /// </summary>
    struct ModelInstance
    {
        ModelAssetHandle modelHandle;
    };

    /// <summary>
    /// Indicates that the model asset with the associated handle failed to load into memory.
    /// </summary>
    struct FailedModelInstance
    {
        ModelAssetHandle modelHandle;
    };
}

LITL_REGISTER_COMPONENT(litl::PendingModelInstance);
LITL_REGISTER_COMPONENT(litl::ModelInstance);
LITL_REGISTER_COMPONENT(litl::FailedModelInstance);

#endif