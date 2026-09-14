#ifndef LITL_ENGINE_ECS_MODEL_INSTANTIATION_SYSTEM_H__
#define LITL_ENGINE_ECS_MODEL_INSTANTIATION_SYSTEM_H__

#include <memory>
#include "litl-engine/ecs/common.hpp"

namespace litl
{
    class AssetManager;
    class ServiceProvider;

    /// <summary>
    /// Polls each frame to check the status of the model referenced by a PendingModelInstance component.
    /// If the model is in memory, then this system replaces the PendingModelInstance component with
    /// a ModelInstance component and creates all requisite other child entities and components (ModelRef, MaterialRef, etc.).
    /// 
    /// The entity with the PendingModelInstance component becomes the root entity for the fully loaded model.
    /// </summary>
    class ModelInstantiationSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Transform const& transform, PendingModelInstance const& pendingModel);

    private:

        std::shared_ptr<AssetManager> m_pAssetManager;
    };
}

#endif