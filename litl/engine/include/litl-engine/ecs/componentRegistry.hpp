#ifndef LITL_ENGINE_ECS_COMPONENT_REGISTRY_H__
#define LITL_ENGINE_ECS_COMPONENT_REGISTRY_H__

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
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