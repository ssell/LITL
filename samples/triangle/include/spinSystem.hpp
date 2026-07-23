#ifndef LITL_SAMPLES_TRIANGLE_SPIN_H__
#define LITL_SAMPLES_TRIANGLE_SPIN_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"

namespace litl
{
    struct Spin 
    {
        vec3 euler = vec3::up();
        float rate = 1.0f;
    };

    struct SpinSystem
    {
        void setup(ServiceProvider& services) {}
        void prepare() {}

        void update(EntityCommands& commands, float dt, Entity entity, Transform& transform, Spin const& spin)
        {
            quat rotation = transform.getRotation();
            rotation *= quat::fromEuler(spin.euler * spin.rate * dt);
            transform.setRotation(rotation);
        }
    };
}

LITL_REGISTER_COMPONENT(litl::Spin);

#endif