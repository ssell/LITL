#ifndef LITL_TESTS_ECS_COMMON_H__
#define LITL_TESTS_ECS_COMMON_H__

#include "litl-core/math/math.hpp"
#include "litl-ecs/world.hpp"

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

#endif