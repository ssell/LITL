#ifndef LITL_SAMPLES_BOIDS_MOVEMENT_H__
#define LITL_SAMPLES_BOIDS_MOVEMENT_H__

#include "components.hpp"

namespace litl
{

    class MovementSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Movement& movement, Transform& transform, Acceleration const& acceleration);

    private:
    };
}

#endif