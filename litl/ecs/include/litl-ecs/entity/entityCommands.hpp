#ifndef LITL_ECS_ENTITY_COMMANDS_H__
#define LITL_ECS_ENTITY_COMMANDS_H__

#include <memory>
#include "litl-ecs/entity/entityCommandQueue.hpp"

namespace litl
{
    /// <summary>
    /// Deferred commands for structural entity changes during system execution.
    /// These commands are performed at the next sync point, which is at the end of the currently running intergroup system layer.
    /// </summary>
    class EntityCommands
    {
    public:

        EntityCommands();
        ~EntityCommands();

        EntityCommands(EntityCommands const&) = delete;
        EntityCommands& operator=(EntityCommands const&) = delete;

        /// <summary>
        /// Resets the command buffer.
        /// </summary>
        void reset() noexcept;

        /// <summary>
        /// Returns the total of number of awaiting commands (both EntityCommand and DeferredEntityCommand).
        /// This does not include the number of "create entity" commands as they effectively vanish upon materialization.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] size_t actionableCommandCount() const noexcept;

        /// <summary>
        /// Materializes the DeferredEntityCommands into EntityCommands, inserts them into the provided
        /// vector at the specified starting offset, and then resets the command buffer state.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="commands"></param>
        /// <param name="offset"></param>
        void extractCommands(World* world, std::vector<EntityCommand>& commands, size_t offset) noexcept;

        /// <summary>
        /// Creates a temporary DeferredEntity.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] DeferredEntity createEntity() noexcept;

        /// <summary>
        /// Inserts a command to destroy the specified Entity.
        /// </summary>
        /// <param name="entity"></param>
        void destroyEntity(Entity entity) noexcept;

        /// <summary>
        /// Inserts a command to destroy the specified DeferredEntity.
        /// </summary>
        /// <param name="entity"></param>
        void destroyEntity(DeferredEntity entity) noexcept;

        /// <summary>
        /// Inserts a command to add the specified component type onto the Entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        void addComponent(Entity entity, ComponentTypeId component) noexcept;

        /// <summary>
        /// Inserts a command to add the specified component type onto the DeferredEntity.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        void addComponent(DeferredEntity entity, ComponentTypeId component) noexcept;

        /// <summary>
        /// Inserts a command to add the specified component type onto the Entity.
        /// The provided shared data must persist until at least the command is run and the data can be copied.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        /// <param name="sharedData"></param>
        void addComponent(Entity entity, ComponentTypeId component, void* sharedData) noexcept;

        /// <summary>
        /// Inserts a command to add the specified component type onto the DeferredEntity.
        /// The provided shared data must persist until at least the command is run and the data can be copied.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        /// <param name="sharedData"></param>
        void addComponent(DeferredEntity entity, ComponentTypeId component, void* sharedData) noexcept;

        /// <summary>
        /// Inserts a command to add the specified component type onto the Entity.
        /// Copies the provided local data into the command buffer's temporary internal memory store.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        /// <param name="localData"></param>
        /// <param name="size"></param>
        /// <param name="alignment"></param>
        void addComponent(Entity entity, ComponentTypeId component, void* localData, size_t size, size_t alignment) noexcept;

        /// <summary>
        /// Inserts a command to add the specified component type onto the DeferredEntity.
        /// Copies the provided local data into the command buffer's temporary internal memory store.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        /// <param name="localData"></param>
        /// <param name="size"></param>
        /// <param name="alignment"></param>
        void addComponent(DeferredEntity entity, ComponentTypeId component, void* localData, size_t size, size_t alignment) noexcept;

        /// <summary>
        /// Inserts a command to add the specified component type onto the Entity.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="entity"></param>
        template<ValidComponentType T>
        void addComponent(Entity entity)
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id);
        }

        /// <summary>
        /// Inserts a command to add the specified component type onto the Entity.
        /// Copies the provided component data into the command buffer's temporary internal memory store.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        template<ValidComponentType T>
        void addComponent(Entity entity, T component) noexcept
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id, &component, sizeof(T), alignof(T));
        }

        /// <summary>
        /// Inserts a command to add the specified component type onto the DeferredEntity.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="entity"></param>
        template<ValidComponentType T>
        void addComponent(DeferredEntity entity) noexcept
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id);
        }

        /// <summary>
        /// Inserts a command to add the specified component type onto the DeferredEntity.
        /// Copies the provided component data into the command buffer's temporary internal memory store.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        template<ValidComponentType T>
        void addComponent(DeferredEntity entity, T component) noexcept
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id, &component, sizeof(T), alignof(T));
        }

        /// <summary>
        /// Inserts a command to remove the specified component type from the Entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        void removeComponent(Entity entity, ComponentTypeId component) noexcept;

        /// <summary>
        /// Inserts a command to remove the specified component type from the DeferredEntity.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        void removeComponent(DeferredEntity entity, ComponentTypeId component) noexcept;

        /// <summary>
        /// Inserts a command to remove the specified component type from the Entity.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="entity"></param>
        template<ValidComponentType T>
        void removeComponent(Entity entity) noexcept
        {
            removeComponent(entity, ComponentDescriptor::get<T>()->id);
        }

        /// <summary>
        /// Inserts a command to remove the specified component type from the DeferredEntity.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="component"></param>
        template<ValidComponentType T>
        void removeComponent(DeferredEntity entity) noexcept
        {
            removeComponent(entity, ComponentDescriptor::get<T>()->id);
        }

        void setParent(Entity entity, Entity parent) noexcept;
        void setParent(Entity entity, DeferredEntity parent) noexcept;
        void setParent(DeferredEntity entity, Entity parent) noexcept;
        void setParent(DeferredEntity entity, DeferredEntity parent) noexcept;

        void removeParent(Entity entity) noexcept;
        void removeParent(DeferredEntity entity) noexcept;

    protected:

    private:

        /// <summary>
        /// Transforms all DeferredEntities into Entities.
        /// </summary>
        /// <param name="world"></param>
        void materialize(World* world) noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif