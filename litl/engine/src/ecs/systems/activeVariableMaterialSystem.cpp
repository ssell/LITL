#include "litl-engine/ecs/systems/activeVariableMaterialSystem.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/material/material.hpp"

namespace litl
{
    void ActiveVariableMaterialSystem::setup(ServiceProvider& services)
    {
        m_pObjectPool = services.get<ObjectPool>();
    }

    void ActiveVariableMaterialSystem::prepare()
    {
        // ... no action ...
    }

    void ActiveVariableMaterialSystem::update(SystemData const& data, Entity entity, VariableMaterialsRef const& materialRef)
    {
        MaterialBindings* materialBindings = m_pObjectPool->getMaterialBindings(materialRef.materialBindingsHandle);

        if (materialBindings != nullptr)
        {
            // Mark the bindings object as a whole active
            materialBindings->setLastActiveFrame(data.frameIndex);

            // Mark the individual material slot bindings active
            auto const& bindings = materialBindings->getBindings();

            for (auto& binding : bindings)
            {
                if (Material* material = m_pObjectPool->getMaterial(binding.handle); material != nullptr)
                {
                    material->markActive(binding.slot);
                }
            }
        }
    }
}