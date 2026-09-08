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
        MaterialBindings* materialBindings = m_pObjectPool->getMaterialBindings(materialRef.materialBindingsHandle);

        if (materialBindings != nullptr)
        {
            auto const& bindings = materialBindings->getBindings();

            for (auto& binding : bindings)
            {
                if (Material* material = m_pObjectPool->getMaterial(binding.handle); material != nullptr)
                {
                    material->markActive({}, binding.slot);
                }
            }
        }
    }
}