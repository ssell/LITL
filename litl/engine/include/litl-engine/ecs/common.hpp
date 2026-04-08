#ifndef LITL_ENGINE_ECS_COMMON_H__
#define LITL_ENGINE_ECS_COMMON_H__

// Include the minimum headers needed for an ECS system

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"

// Include common, widely used components

#include "litl-engine/ecs/components/transform.hpp"

/*

Minimal ECS System definition:
 
struct System 
{
    void setup(Core::ServiceProvider& services) {}
    void update(ECS::EntityCommands& commands, float dt) {}
};

 */

#endif