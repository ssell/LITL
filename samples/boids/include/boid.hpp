#ifndef LITL_SAMPLES_BOIDS_BOID_H__
#define LITL_SAMPLES_BOIDS_BOID_H__

#include "litl-core/math/types.hpp"
#include "litl-ecs/register.hpp"

namespace litl
{
    enum class BoidState : uint8_t
    {
        Idle = 0u,
        Traveling = 1u,
        Fleeing = 2
    };

    struct Boid
    {
        /// <summary>
        /// The current state of this boid.
        /// </summary>
        BoidState stage{ BoidState::Idle };

        /// <summary>
        /// The fixed target that the boid is currently travelling to.
        /// </summary>
        vec3 target{};
    };
}

LITL_REGISTER_COMPONENT(litl::Boid);

#endif