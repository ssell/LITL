#ifndef LITL_ECS_ENTITY_COMMANDS_H__
#define LITL_ECS_ENTITY_COMMANDS_H__

#include "litl-ecs/entity/entityCommandQueue.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// Deferred commands for structural entity changes during system execution.
    /// These commands are performed at the next sync point, which is at the end of
    /// the currently running intergroup system layer.
    /// </summary>
    class EntityCommands
    {
    public:

        EntityCommands();
        ~EntityCommands();

        EntityCommands(EntityCommands const&) = delete;
        EntityCommands& operator=(EntityCommands const&) = delete;

        /// <summary>
        /// 
        /// </summary>
        void reset() noexcept;

        /// <summary>
        /// Returns the total of number of awaiting commands (both EntityCommand and DeferredEntityCommand).
        /// This does not include the number of "create entity" commands as they effectively vanish upon materialization.
        /// </summary>
        /// <returns></returns>
        size_t actionableCommandCount() const noexcept;

        /// <summary>
        /// Materializes the DeferredEntityCommands into EntityCommands, inserts them into the provided
        /// vector at the specified starting offset, and then resets the command buffer state.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="commands"></param>
        /// <param name="offset"></param>
        void extractCommands(World* world, std::vector<EntityCommand>& commands, size_t offset) noexcept;

        [[nodiscard]] DeferredEntity createEntity() noexcept;
        void destroyEntity(Entity entity) noexcept;
        void destroyEntity(DeferredEntity entity) noexcept;
        void addComponent(Entity entity, ComponentTypeId component, void* data = nullptr) noexcept;
        void addComponent(DeferredEntity entity, ComponentTypeId component, void* data = nullptr) noexcept;
        void removeComponent(Entity entity, ComponentTypeId component) noexcept;
        void removeComponent(DeferredEntity entity, ComponentTypeId component) noexcept;

        template<ValidComponentType T>
        void addComponent(Entity entity)
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id);
        }

        template<ValidComponentType T>
        void addComponent(Entity entity, T const& component) noexcept
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id, component);
        }

        template<ValidComponentType T>
        void addComponent(DeferredEntity entity) noexcept
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id);
        }

        template<ValidComponentType T>
        void addComponent(DeferredEntity entity, T const& component) noexcept
        {
            addComponent(entity, ComponentDescriptor::get<T>()->id, component);
        }

        template<ValidComponentType T>
        void removeComponent(Entity entity) noexcept
        {
            removeComponent(entity, ComponentDescriptor::get<T>()->id);
        }

        template<ValidComponentType T>
        void removeComponent(DeferredEntity entity) noexcept
        {
            removeComponent(entity, ComponentDescriptor::get<T>()->id);
        }

    protected:

    private:

        void materialize(World* world) noexcept;

        EntityCommandQueue m_commands;
        uint32_t m_nextId;
    };
}

#endif