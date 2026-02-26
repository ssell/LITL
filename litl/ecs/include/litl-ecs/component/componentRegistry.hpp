#ifndef LITL_ENGINE_ECS_COMPONENT_REGISTRY_H__
#define LITL_ENGINE_ECS_COMPONENT_REGISTRY_H__

#include "litl-ecs/constants.hpp"
#include "litl-ecs/component/component.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// Used for maintaining mappings of stable id <-> descriptor and runtime (not templated) component resolution.
    /// </summary>
    class ComponentRegistry
    {
    public:

        static void track(ComponentDescriptor const* descriptor) noexcept;
        static ComponentDescriptor const* find(ComponentTypeId unstableId) noexcept;
        static ComponentDescriptor const* findByStableId(StableComponentTypeId stableId) noexcept;

    protected:

    private:

    };
}

#endif