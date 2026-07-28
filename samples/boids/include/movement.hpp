#ifndef LITL_SAMPLES_BOIDS_MOVEMENT_H__
#define LITL_SAMPLES_BOIDS_MOVEMENT_H__

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"

namespace litl
{
    struct Movement
    {
        vec3 direction{};
        float speed{ 0.0f };
    };

    class MovementSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Movement const& movement, Transform& transform);

    private:
    };
}

LITL_REGISTER_COMPONENT(litl::Movement);

#endif