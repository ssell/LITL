#include "litl-ecs/entity/entityCommands.hpp"

namespace LITL::ECS
{
    EntityCommands::EntityCommands()
    {

    }

    EntityCommands::~EntityCommands()
    {

    }

    DeferredEntity EntityCommands::createEntity() noexcept
    {
        return {};
    }

    void EntityCommands::destroyEntity(Entity entity) noexcept
    {

    }

    void EntityCommands::destroyEntity(DeferredEntity entity) noexcept
    {

    }

    void EntityCommands::addComponent(Entity entity, ComponentTypeId component, void* data) noexcept
    {

    }

    void EntityCommands::addComponent(DeferredEntity entity, ComponentTypeId component, void* data) noexcept
    {

    }

    void EntityCommands::removeComponent(Entity entity, ComponentTypeId component) noexcept
    {

    }

    void EntityCommands::removeComponent(DeferredEntity entity, ComponentTypeId component) noexcept
    {

    }
}