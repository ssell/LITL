#ifndef LITL_ECS_COMPONENT_DATA_H__
#define LITL_ECS_COMPONENT_DATA_H__

#include "litl-ecs/constants.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// Combination of a component type along with the memory address of where the source data is located.
    /// Used for when performing an "add + set" component operation at the same time.
    /// </summary>
    struct ComponentData
    {
        ComponentTypeId type{ 0 };
        void* data{ nullptr };
    };
}

#endif