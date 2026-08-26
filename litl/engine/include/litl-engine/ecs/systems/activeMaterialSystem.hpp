#ifndef LITL_ENGINE_ECS_ACTIVE_MATERIAL_SYSTEM_H__
#define LITL_ENGINE_ECS_ACTIVE_MATERIAL_SYSTEM_H__

#include <memory>
#include "litl-engine/render/renderableEntity.hpp"

namespace litl
{
    class ServiceProvider;
    class ObjectPool;

    /// <summary>
    /// Responsible for marking all MaterialRefs attached to entities as active.
    /// Any material slot that is not active will be removed after a number of frames.
    /// </summary>
    class ActiveMaterialSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, MaterialRef const& materialRef);

    private:

        std::shared_ptr<ObjectPool> m_pObjectPool{ nullptr };
    };
}

#endif