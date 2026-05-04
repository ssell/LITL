#ifndef LITL_ECS_ENTITY_COMMAND_H__
#define LITL_ECS_ENTITY_COMMAND_H__

#include <cstdint>
#include <optional>

#include "litl-core/constants.hpp"
#include "litl-ecs/constants.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-ecs/entity/deferredEntity.hpp"
#include "litl-ecs/component/component.hpp"

namespace litl
{
    class EntityCommandQueue;

    enum class EntityCommandType : uint32_t
    {
        None            = 0,
        CreateEntity    = 1,
        DestroyEntity   = 2,
        AddComponent    = 3,
        RemoveComponent = 4,
        SetParent       = 5
    };

    // Ordering is important as it is used in command buffer sorting
    static_assert(EntityCommandType::CreateEntity < EntityCommandType::DestroyEntity);
    static_assert(EntityCommandType::DestroyEntity < EntityCommandType::AddComponent);
    static_assert(EntityCommandType::AddComponent < EntityCommandType::RemoveComponent);
    static_assert(EntityCommandType::RemoveComponent < EntityCommandType::SetParent);

    /// <summary>
    /// Data used during an AddComponent or RemoveComponent command.
    /// </summary>
    struct ComponentCommandInfo
    {
        /// <summary>
        /// The component being added or removed.
        /// </summary>
        ComponentTypeId component{ ecs::Constants::null_component_id };

        /// <summary>
        /// The address to the component data being added.
        /// </summary>
        void* data{ nullptr };
    };

    /// <summary>
    /// Data used during a SetParent command.
    /// The desired parent could be an Entity, DeferredEntity, or null (sever parentage).
    /// </summary>
    struct SetParentCommandInfo
    {
        Entity parent{};
        DeferredEntity deferredParent{};
    };

    /// <summary>
    /// A command related to an Entity.
    /// </summary>
    struct EntityCommand
    {
        /// <summary>
        /// The command to be run.
        /// </summary>
        EntityCommandType type{ EntityCommandType::None };

        /// <summary>
        /// The entity being targetted by the command.
        /// </summary>
        Entity entity{};

        /// <summary>
        /// If an AddComponent or RemoveComponent command, the relevant info.
        /// </summary>
        ComponentCommandInfo componentInfo{};

        /// <summary>
        /// If a SetParent command, the relevant info.
        /// </summary>
        SetParentCommandInfo setParentInfo{};

        /// <summary>
        /// The queue that stores the local component.
        /// </summary>
        EntityCommandQueue const* queue{ nullptr };
    };

    /// <summary>
    /// A command related to a DeferredEntity.
    /// </summary>
    struct DeferredEntityCommand
    {
        /// <summary>
        /// The command to be run.
        /// </summary>
        EntityCommandType type{ EntityCommandType::None };

        /// <summary>
        /// Optional deferred entity being targetted by the command.
        /// If this is set, then the command is for an entity that does not yet exist.
        /// </summary>
        DeferredEntity deferredEntity{};

        /// <summary>
        /// If an AddComponent or RemoveComponent command, the relevant info.
        /// </summary>
        ComponentCommandInfo componentInfo{};

        /// <summary>
        /// If a SetParent command, the relevant info.
        /// </summary>
        SetParentCommandInfo setParentInfo{};
    };
}

#endif