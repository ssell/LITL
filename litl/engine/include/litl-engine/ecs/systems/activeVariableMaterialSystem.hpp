#ifndef LITL_ENGINE_ECS_ACTIVE_VARIABLE_MATERIAL_SYSTEM_H__
#define LITL_ENGINE_ECS_ACTIVE_VARIABLE_MATERIAL_SYSTEM_H__

#include <memory>

#include "litl-ecs/system/systemData.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"

namespace litl
{
    class ServiceProvider;
    class ObjectPool;

    /// <summary>
    /// Responsible for marking all VariableMaterialsRefs attached to entities as active.
    /// Any VariableMaterialsRef that is not active will be removed after a number of frames.
    /// </summary>
    class ActiveVariableMaterialSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, VariableMaterialsRef const& materialRef);

    private:

        std::shared_ptr<ObjectPool> m_pObjectPool{ nullptr };
    };
}

#endif