#ifndef LITL_SAMPLES_BOIDS_CONTROLS_H__
#define LITL_SAMPLES_BOIDS_CONTROLS_H__

#include "litl-core/math/types.hpp"

namespace litl::samples
{
    /// <summary>
    /// All of the control parameters for flocking.
    /// </summary>
    struct FlockControls
    {
        float separationRadius = 20.0f;
        float perceptionRadius = 25.0f;
        float separationWeight = 3.0f;
        float alignmentWeight = 2.0f;
        float cohesionWeight = 1.0f;
        float targetWeight = 2.0f;
    };

    /// <summary>
    /// All the control parameters for steering.
    /// </summary>
    struct SteeringControls
    {
        float maxSpeed = 1.0f;
        float maxForce = 1.0f;
    };

    static constexpr FlockControls g_boidFlocking = FlockControls{};
    static constexpr SteeringControls g_boidSteering = SteeringControls{ .maxSpeed = 100.0f, .maxForce = 100.0f  };
    static constexpr SteeringControls g_predatorSteering = SteeringControls{ .maxSpeed = 200.0f, .maxForce = 200.0f };

    [[nodiscard]] inline vec3 steerTowards(vec3 desired, vec3 velocity, SteeringControls const& controls) noexcept
    {
        const float lengthSq = desired.lengthSquared();

        if (lengthSq < Traits<float>::epsilon)
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