#ifndef LITL_ECS_SYSTEM_COLLECTION_CONTEXT_H__
#define LITL_ECS_SYSTEM_COLLECTION_CONTEXT_H__

#include "litl-ecs/system/systemRegistry.hpp"
#include "litl-ecs/system/systemPlacementHint.hpp"

namespace LITL::ECS
{
    class SystemCollection;
    class System;

    /// <summary>
    /// Utility to provide a builder-style interface for system dependencies.
    /// </summary>
    class SystemCollectionContext
    {
    public:

        SystemCollectionContext(SystemCollection const* collection, System const* system);

        template<ValidSystem S>
        SystemCollectionContext const& dependsOn() const noexcept
        {
            return dependsOn(SystemRegistry::getSystem<S>());
        }

        SystemCollectionContext const& placement(SystemPlacementHint hint) const noexcept;

    protected:

    private:

        SystemCollectionContext const& dependsOn(System* system) const noexcept;

        SystemCollection const* m_pCollection;
        System const* m_pSystem;
    };
}

#endif