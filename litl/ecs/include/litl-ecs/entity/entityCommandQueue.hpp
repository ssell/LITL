#ifndef LITL_ECS_ENTITY_COMMAND_QUEUE_H__
#define LITL_ECS_ENTITY_COMMAND_QUEUE_H__

#include <memory>
#include <optional>
#include <vector>

#include "litl-ecs/entity/entityCommand.hpp"

namespace litl
{
    class World;

    /// <summary>
    /// A double-queue of entity commands: one for entities, one for deferred entities.
    /// </summary>
    class EntityCommandQueue
    {
    public:

        EntityCommandQueue();
        ~EntityCommandQueue();

        EntityCommandQueue(EntityCommandQueue const&) = delete;
        EntityCommandQueue& operator=(EntityCommandQueue const&) = delete;

        /// <summary>
        /// Adds a new commmand for an Entity.
        /// </summary>
        /// <param name="command"></param>
        /// <param name="data"></param>
        void push(EntityCommand command, void* source = nullptr) noexcept;

        /// <summary>
        /// Adds a new command for a DeferredEntity.
        /// </summary>
        /// <param name="command"></param>
        /// <param name="data"></param>
        void push(DeferredEntityCommand command, void* source = nullptr) noexcept;

        /// <summary>
        /// Returns the next command. Note this does not remove it, that is only done in a call to reset.
        /// </summary>
        /// <returns></returns>
        std::optional<EntityCommand> next() noexcept;

        /// <summary>
        /// Transfers the local component data to the specified destination address.
        /// </summary>
        /// <param name="command"></param>
        /// <param name="dest"></param>
        /// <returns></returns>
        bool loadComponent(EntityCommand command, void* dest) noexcept;

        /// <summary>
        /// Number of entity commands left to see with next.
        /// </summary>
        /// <returns></returns>
        size_t count() const noexcept;

        /// <summary>
        /// Number of non-create commands.
        /// </summary>
        /// <returns></returns>
        size_t actionableCommandCount() const noexcept;

        /// <summary>
        /// Number of component data pools.
        /// </summary>
        /// <returns></returns>
        size_t poolCount() const noexcept;

        /// <summary>
        /// Is the queue out of commands to see?
        /// </summary>
        /// <returns></returns>
        bool empty() const noexcept;

        /// <summary>
        /// Resets the queue back to a default state.
        /// Pools remain, but their offsets are reset.
        /// </summary>
        void reset() noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        std::vector<DeferredEntityCommand> const& deferredCommands() const noexcept;

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif