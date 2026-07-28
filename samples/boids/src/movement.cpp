#include "movement.hpp"

namespace litl
{
    void MovementSystem::setup(ServiceProvider& services)
    {
        // ... intentionally empty ...
    }

    void MovementSystem::prepare()
    {
        // ... intentionally empty ...
    }

    void MovementSystem::update(SystemData const& data, Entity entity, Boid const& boid, Movement& movement, Transform& transform)
    {
        const vec3 prevPosition = transform.getPosition();
        const vec3 prevVelocity = movement.velocity;

        movement.velocity = truncate(movement.velocity + (boid.acceleration * data.deltaTime), boid.maxSpeed);

        if (movement.velocity.lengthSquared() > 0.0f)
        {
            const vec3 lookAtDir = lerp(prevVelocity, movement.velocity, data.deltaTime).normalized();

            transform.translate(movement.velocity * data.deltaTime);
            transform.setRotation(quat::lookRotation(lookAtDir, vec3::up()));
        }
    }
}