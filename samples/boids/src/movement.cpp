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

    void MovementSystem::update(SystemData const& data, Entity entity, Movement const& movement, Transform& transform)
    {
        if (movement.direction.isNormalized())
        {
            transform.translate(movement.direction * movement.speed * data.deltaTime);
        }
    }
}