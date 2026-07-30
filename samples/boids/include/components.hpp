#ifndef LITL_SAMPLES_BOIDS_COMPONENTS_H__
#define LITL_SAMPLES_BOIDS_COMPONENTS_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"
#include "controls.hpp"

namespace litl::samples
{
    /// <summary>
    /// The primary state of an individual boid.
    /// </summary>
    struct Boid
    {
        /// <summary>
        /// The fixed target that the boid is currently travelling to.
        /// </summary>
        vec3 target{};

        /// <summary>
        /// Which system update phase are we part of?
        /// </summary>
        uint32_t phase{ 0u };

        /// <summary>
        /// The last time tick was called for this boid.
        /// </summary>
        float lastTick{ 0.0f };

        /// <summary>
        /// Are we currently moving away from a predator?
        /// </summary>
        bool isFleeing{ false };
    };

    /// <summary>
    /// The primary state for an individual predator.
    /// </summary>
    struct Predator
    {
        /// <summary>
        /// The fixed target that the predator is currently travelling to.
        /// </summary>
        vec3 target{};

        /// <summary>
        /// Fixed index for the predator which is used to update its position
        /// with the simulator in a thread-safe manny without sync mechanisms.
        /// </summary>
        uint32_t index{ 0u };

        /// <summary>
        /// The last time tick was called for this predator.
        /// </summary>
        float lastTick{ 0.0f };

        /// <summary>
        /// Are we currently moving to a target?
        /// </summary>
        bool movingToTarget{ false };
    };

    /// <summary>
    /// Primary state for an individual food.
    /// </summary>
    struct Food
    {
        enum class Status
        {
            None = 0,
            Alive = 1,
            Eaten = 2
        };

        /// <summary>
        /// Index into the simulators tracking pool for food.
        /// </summary>
        uint32_t index{ 0u };

        /// <summary>
        /// The last time tick was called for this food.
        /// </summary>
        float lastTick{ 0.0f };
    };

    /// <summary>
    /// The current acceleration for an entity, along with its maximum speed.
    /// 
    /// This is split apart from Movement as boid flocking relies on the velocity (from movement)
    /// as part of its alignment force, but it is not safe to read and write the component of 
    /// another entity at the same time.
    /// </summary>
    struct Acceleration
    {
        vec3 acceleration{};
        float maxSpeed{ 0.0f };
    };

    /// <summary>
    /// The current velocity for an entity.
    /// </summary>
    struct Movement
    {
        vec3 velocity;
    };
}

LITL_REGISTER_COMPONENT(litl::samples::Boid);
LITL_REGISTER_COMPONENT(litl::samples::Predator);
LITL_REGISTER_COMPONENT(litl::samples::Food);
LITL_REGISTER_COMPONENT(litl::samples::Movement);
LITL_REGISTER_COMPONENT(litl::samples::Acceleration);

#endif