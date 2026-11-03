#ifndef LITL_ENGINE_RENDER_RENDERABLE_ENTITY_H__
#define LITL_ENGINE_RENDER_RENDERABLE_ENTITY_H__

#include <cstdint>

#include "litl-core/math/types/vec3.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/meshRef.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/objects/material/material.hpp"

namespace litl
{
    /// <summary>
    /// Minimal data needed to render an entity.
    /// </summary>
    struct RenderableEntity
    {
        /// <summary>
        /// The ECS entity being rendered.
        /// </summary>
        Entity entity{};

        /// <summary>
        /// The mesh being rendered.
        /// </summary>
        MeshRef meshRef{};

        /// <summary>
        /// The material being applied to the submesh denoted by the mesh and start index + index count.
        /// </summary>
        MaterialRef materialRef{};

        /// <summary>
        /// The blend mode of the material (opaque, mask, or transparent).
        /// </summary>
        MaterialBlendMode materialBlendMode{};

        /// <summary>
        /// Position of the entity in world-space.
        /// </summary>
        vec3 position{};

        /// <summary>
        /// Squared distance from the camera that this renderable is associated with.
        /// </summary>
        float distanceSquared{ 0.0f };

        /// <summary>
        /// The first index into the mesh that is being rendered.
        /// </summary>
        uint32_t firstIndex{ 0u };

        /// <summary>
        /// The number of indices being rendered, offset by the firstIndex.
        /// If this is set to Constants::uint32_null_index (~0) then all indices from the firstIndex are to be rendered.
        /// </summary>
        uint32_t indexCount{ 0u };
    };
}

#endif