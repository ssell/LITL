#ifndef LITL_SAMPLES_BOID_FOOD_H__
#define LITL_SAMPLES_BOID_FOOD_H__

#include "litl-ecs/register.hpp"

namespace litl
{
    struct Food
    {
        // Components can be empty and serve as flags.
    };
}

LITL_REGISTER_COMPONENT(litl::Food);

#endif