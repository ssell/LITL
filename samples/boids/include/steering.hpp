#ifndef LITL_SAMPLES_BOIDS_STEERING_H__
#define LITL_SAMPLES_BOIDS_STEERING_H__

#include "litl-core/math/types.hpp"

namespace litl
{
    struct SteeringControls
    {
        float separationRadius = 20.0f;
        float perceptionRadius = 35.0f;
        float cosFov = -0.5f;
        float separationWeight = 3.0f;
        float alignmentWeight = 2.0f;
        float cohesionWeight = 1.0f;
        float targetWeight = 1.5f;
        float maxSpeed = 100.0f;
        float maxForce = 100.0f;
    };

    static constexpr SteeringControls g_boidSteering = SteeringControls{};
    static constexpr SteeringControls g_predatorSteering = SteeringControls{};

    inline constexpr float kEpsilonSq = 1e-8f;

    [[nodiscard]] inline vec3 steerTowards(vec3 desired, vec3 velocity, SteeringControls const& controls) noexcept
    {
        const float lengthSq = desired.lengthSquared();

        if (lengthSq < kEpsilonSq)
        {
            return vec3{};
        }
        else
        {
            const vec3 target = desired * (controls.maxSpeed / sqrtf(lengthSq));
            return truncate(target - velocity, controls.maxForce);
        }
    }
}

#endif