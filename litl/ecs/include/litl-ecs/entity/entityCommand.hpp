#ifndef LITL_ECS_ENTITY_COMMAND_H__
#define LITL_ECS_ENTITY_COMMAND_H__

#include <cstdint>
#include <optional>

#include "litl-ecs/entity/entity.hpp"
#include "litl-ecs/entity/deferredEntity.hpp"
#include "litl-ecs/component/component.hpp"

namespace LITL::ECS
{
    enum EntityCommandType : uint32_t
    {
        None            = 0,
        CreateEntity    = 1,
        DestroyEntity   = 2,
        AddComponent    = 3,
        RemoveComponent = 4
    };

    /// <summary>
    /// A single deferred entity-related command.
    /// </summary>
    struct EntityCommand
    {
        /// <summary>
        /// The command to be run.
        /// </summary>
        EntityCommandType type{ EntityCommandType::None };

        /// <summary>
        /// If the enttiy being targetted by the command.
        /// </summary>
        Entity entity;

        /// <summary>
        /// Optional deferred entity being targetted by the command.
        /// If this is set, then the command is for an entity that does not yet exist.
        /// </summary>
        DeferredEntity deferredEntity{};

        /// <summary>
        /// The component being added or removed.
        /// </summary>
        ComponentTypeId component;

        /// <summary>
        /// Index of the memory page in which the component data is stored.
        /// </summary>
        uint32_t page{ 0 };

        /// <summary>
        /// Offset into the memory page, pointing to where the component data is stored.
        /// </summary>
        uint32_t offset{ 0 };
    };
}

#endif