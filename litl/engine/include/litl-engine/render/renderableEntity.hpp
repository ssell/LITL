#ifndef LITL_ENGINE_RENDER_RENDERABLE_ENTITY_H__
#define LITL_ENGINE_RENDER_RENDERABLE_ENTITY_H__

#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/meshRef.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"

namespace litl
{
    /// <summary>
    /// Minimal data needed to render an entity.
    /// </summary>
    struct RenderableEntity
    {
        Entity entity{};
        Transform transform{};
        MeshRef mesh{};
        MaterialRef material{};
    };
}

#endif