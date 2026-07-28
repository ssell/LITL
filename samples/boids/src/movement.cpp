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
        movement.velocity = truncate(movement.velocity + (boid.acceleration * data.deltaTime), boid.maxSpeed);
        transform.translate(movement.velocity * data.deltaTime);
    }
}