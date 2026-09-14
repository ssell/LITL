#ifndef LITL_ENGINE_ECS_ACTIVE_MATERIAL_SYSTEM_H__
#define LITL_ENGINE_ECS_ACTIVE_MATERIAL_SYSTEM_H__

#include <memory>

#include "litl-ecs/system/systemData.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/ecs/components/modelInstance.hpp"

/**
 * Note that we have three distinct systems for tracking active materials:
 * 
 *     * ActiveMaterialSystem tracks MaterialRef
 *     * ActiveVariableMaterialSystem tracks VariableMaterialsRef
 *     * PendingModelMaterialSystem tracks PendingModelInstance (for the fallback material)
 * 
 * In theory they could be combined into a single system whose update would be
 * 
 *     update(SystemData const& data, Entity entity, MaterialRef const* materialRef, VariableMaterialRef const* variableMaterialRef, PendingModelInstance const* pendingmodel)
 * 
 * However that would result in iterating over all archetypes and entities. 
 * Since there are no further requirements for a material to be active than the individual material components then we must split into separate systems and have hard requirements on those components.
 */

namespace litl
{
    class ServiceProvider;
    class ObjectPool;

    /// <summary>
    /// Responsible for marking all MaterialRefs attached to entities as active.
    /// Any material slot that is not active will be removed after a number of frames.
    /// </summary>
    class ActiveMaterialSystem final
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, MaterialRef const& materialRef);

    private:

        std::shared_ptr<ObjectPool> m_pObjectPool{ nullptr };
    };

    /// <summary>
    /// Responsible for marking all VariableMaterialsRefs attached to entities as active.
    /// Any VariableMaterialsRef that is not active will be removed after a number of frames.
    /// </summary>
    class ActiveVariableMaterialSystem final
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, VariableMaterialsRef const& materialRef);

    private:

        std::shared_ptr<ObjectPool> m_pObjectPool{ nullptr };
    };

    /// <summary>
    /// Pending models may specify a fallback material handle + slot to use in the event there are meshes without valid materials.
    /// We want to ensure any pre-allocated material slot is kept alive until the model is done loading.
    /// </summary>
    class PendingModelMaterialSystem final
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, PendingModelInstance const& pendingModel);

    private:

        std::shared_ptr<ObjectPool> m_pObjectPool{ nullptr };
    };
}

#endif