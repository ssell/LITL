#include "litl-engine/ecs/systems/activeMaterialSystem.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/material.hpp"

namespace litl
{
    void ActiveMaterialSystem::setup(ServiceProvider& services)
    {
        m_pObjectPool = services.get<ObjectPool>();
    }

    void ActiveMaterialSystem::prepare()
    {
        // ... no action ...
    }

    void ActiveMaterialSystem::update(SystemData const& data, Entity entity, MaterialRef const& materialRef)
    {
        Material* material = m_pObjectPool->getMaterial(materialRef.handle);

        if (material != nullptr)
        {
            material->markActive({}, materialRef.slot);
        }
    }
}