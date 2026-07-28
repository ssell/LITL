#ifndef LITL_SAMPLES_BOIDS_COMPONENTS_H__
#define LITL_SAMPLES_BOIDS_COMPONENTS_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"
#include "steering.hpp"

namespace litl
{
    struct Boid
    {
        /// <summary>
        /// The fixed target that the boid is currently travelling to.
        /// </summary>
        vec3 target{};

        /// <summary>
        /// 
        /// </summary>
        vec3 acceleration{};

        /// <summary>
        /// 
        /// </summary>
        float maxSpeed{};

        /// <summary>
        /// The last time tick was called for this boid.
        /// </summary>
        float lastTick{ 0.0f };
    };

    struct Predator
    {
        // ... todo ...
    };

    struct Food
    {
        // Components can be empty and serve as flags.
    };

    struct Movement
    {
        vec3 velocity;
    };
}

LITL_REGISTER_COMPONENT(litl::Boid);
LITL_REGISTER_COMPONENT(litl::Predator);
LITL_REGISTER_COMPONENT(litl::Food);
LITL_REGISTER_COMPONENT(litl::Movement);

#endif