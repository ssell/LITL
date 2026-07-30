#ifndef LITL_ECS_REGISTER_H__
#define LITL_ECS_REGISTER_H__

#include "litl-core/types.hpp"
#include "litl-ecs/constants.hpp"

// Asserts that T is a valid component and registers its typename which is used as the stable component id.
#define LITL_REGISTER_COMPONENT(T) \
    static_assert(sizeof(T) <= litl::ecs::Constants::max_component_size, "Size of component exceeds maximum allowed size."); \
    static_assert(litl::ValidComponentType<T>, "Component fails ValidComponentType check."); \
    LITL_REGISTER_TYPE_NAME(T)

#endif