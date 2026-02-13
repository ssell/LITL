#ifndef LITL_ENGINE_ECS_ENTITY_H__
#define LITL_ENGINE_ECS_ENTITY_H__

#include <cstdint>

namespace LITL::Engine::ECS
{
    struct Entity
    {
        uint32_t index;
        uint32_t generation;

        bool operator==(Entity const&) const = default;
    };
}

#endif