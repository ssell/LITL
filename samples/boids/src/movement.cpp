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

    void MovementSystem::update(SystemData const& data, Entity entity, Movement& movement, Transform& transform, Acceleration const& acceleration)
    {
        const vec3 prevPosition = transform.getPosition();
        const vec3 prevVelocity = movement.velocity;

        movement.velocity = truncate(movement.velocity + (acceleration.acceleration * data.deltaTime), acceleration.maxSpeed);

        if (movement.velocity.lengthSquared() > 0.0f)
        {
            const vec3 lookAtDir = lerp(prevVelocity, movement.velocity, data.deltaTime).normalized();

            transform.translate(movement.velocity * data.deltaTime);
            transform.setRotation(quat::lookRotation(lookAtDir, vec3::up()));
        }
    }
}