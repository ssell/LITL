#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/ecs/systems/modelInstantiationSystem.hpp"

namespace litl
{
    void ModelInstantiationSystem::setup(ServiceProvider& services)
    {
        m_pAssetManager = services.get<AssetManager>();
    }

    void ModelInstantiationSystem::prepare()
    {

    }

    void ModelInstantiationSystem::update(SystemData const& data, Entity entity, Transform const& transform, PendingModelInstance const& pendingModel)
    {
        // ... todo ...
    }
}