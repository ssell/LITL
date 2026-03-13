#ifndef LITL_TESTS_ECS_COMMON_H__
#define LITL_TESTS_ECS_COMMON_H__

#include "litl-core/types.hpp"
#include "litl-core/math/math.hpp"
#include "litl-ecs/world.hpp"

namespace LITL::ECS::Tests
{
    struct Foo
    {
        uint32_t a;
    };

    struct Bar
    {
        float a;
        uint32_t b;
    };

    struct TestSystem
    {
        void update(LITL::ECS::World& world, float dt, Foo& foo, Bar& bar)
        {
            foo.a++;
            bar.b++;
        }
    };
}

REGISTER_TYPE_NAME(LITL::ECS::Tests::Foo)
REGISTER_TYPE_NAME(LITL::ECS::Tests::Bar)

#endif