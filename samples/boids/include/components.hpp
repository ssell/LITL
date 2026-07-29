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
        uint32_t phase{ 0u };

        /// <summary>
        /// The last time tick was called for this boid.
        /// </summary>
        float lastTick{ 0.0f };

        /// <summary>
        /// 
        /// </summary>
        bool isFleeing{ false };
    };

    struct Predator
    {
        /// <summary>
        /// The fixed target that the predator is currently travelling to.
        /// </summary>
        vec3 target{};

        /// <summary>
        /// 
        /// </summary>
        vec3 acceleration{};

        uint32_t index{ 0u };

        /// <summary>
        /// The last time tick was called for this predator.
        /// </summary>
        float lastTick{ 0.0f };

        bool movingToTarget{ false };
    };

    enum class FoodStatus : uint8_t
    {
        None = 0,
        Alive = 1,
        Eaten = 2
    };

    struct Food
    {
        /// <summary>
        /// Index into the simulators tracking pool for food.
        /// </summary>
        uint32_t index{ 0u };

        /// <summary>
        /// The last time tick was called for this food.
        /// </summary>
        float lastTick{ 0.0f };
    };

    struct Acceleration
    {
        vec3 acceleration{};
        float maxSpeed{ 0.0f };
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
LITL_REGISTER_COMPONENT(litl::Acceleration);

#endif