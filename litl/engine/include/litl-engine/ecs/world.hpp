#ifndef LITL_ENGINE_ECS_WORLD_H__
#define LITL_ENGINE_ECS_WORLD_H__

#include "litl-engine/ecs/entity.hpp"

namespace LITL::Engine::ECS
{
    class World final
    {
    public:

        World();
        World(World const&) = delete;
        World& operator=(World const&) = delete;
        ~World();

        Entity create() noexcept;

    protected:

    private:
    };
}

#endif