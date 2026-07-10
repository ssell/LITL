#ifndef LITL_ECS_REGISTER_H__
#define LITL_ECS_REGISTER_H__

#include "litl-core/types.hpp"
#include "litl-ecs/constants.hpp"

// Asserts that T is a valid component and registers its typename which is used as the stable component id.
#define LITL_REGISTER_COMPONENT(T) \
    static_assert(litl::ValidComponentType<T>); \
    LITL_REGISTER_TYPE_NAME(T)

#endif