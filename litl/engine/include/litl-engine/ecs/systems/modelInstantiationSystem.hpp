#ifndef LITL_ENGINE_ECS_MODEL_INSTANTIATION_SYSTEM_H__
#define LITL_ENGINE_ECS_MODEL_INSTANTIATION_SYSTEM_H__

#include <memory>
#include "litl-engine/ecs/common.hpp"

namespace litl
{
    class AssetManager;
    class ServiceProvider;

    /// <summary>
    /// Polls each frame to check the status of the model asset referenced by a PendingModelInstance component.
    /// If the model is in memory, then this system replaces the PendingModelInstance component with
    /// a ModelInstance component and creates all requisite other child entities and components (ModelRef, MaterialRef, etc.).
    /// 
    /// The entity with the PendingModelInstance component becomes the root entity for the fully loaded model.
    /// If the entity does not already have a transform, an identity transform will be added to it upon successful model load.
    /// 
    /// If the asset failed to load then the PendingModelInstance is replaced with a FailedModelInstance component.
    /// </summary>
    class ModelInstantiationSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Transform const* transform, PendingModelInstance const& pendingModel, LocalBounds* localBounds);

    private:

        std::shared_ptr<AssetManager> m_pAssetManager;
    };
}

#endif