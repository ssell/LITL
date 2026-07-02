#ifndef LITL_ENGINE_ECS_COMPONENTS_SCALE_H__
#define LITL_ENGINE_ECS_COMPONENTS_SCALE_H__

#include "litl-core/math.hpp"
#include "litl-ecs/register.hpp"

namespace litl
{
    /// <summary>
    /// A non-uniform scale applied to an entity. By default, the Transform component only stores an uniform scale.
    /// 
    /// This is to reduce the memory size of the majority of entities - 8 fewer bytes over potentially hundreds 
    /// of thousands of entities adds up. Non-uniform scaling also behaves differently in regards to rotation.
    /// </summary>
    struct NonUniformScale
    {
        vec3 scale{ 1.0f, 1.0f, 1.0f };
    };
}

LITL_REGISTER_COMPONENT(litl::NonUniformScale);

#endif