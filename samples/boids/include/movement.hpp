#ifndef LITL_SAMPLES_BOIDS_MOVEMENT_H__
#define LITL_SAMPLES_BOIDS_MOVEMENT_H__

#include "components.hpp"

namespace litl::samples
{
    /// <summary>
    /// Given an acceleration, performs translation and rotation on the entity.
    /// Stores the current velocity onto the Movement component.
    /// </summary>
    class MovementSystem final
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Movement& movement, Transform& transform, Acceleration const& acceleration);

    private:
    };
}

#endif