#ifndef LITL_ECS_ENTITY_COMMAND_QUEUE_H__
#define LITL_ECS_ENTITY_COMMAND_QUEUE_H__

#include <memory>
#include <optional>

#include "litl-ecs/entity/entityCommand.hpp"

namespace LITL::ECS
{
    class EntityCommandQueue
    {
    public:

        EntityCommandQueue();
        ~EntityCommandQueue();

        EntityCommandQueue(EntityCommandQueue const&) = delete;
        EntityCommandQueue& operator=(EntityCommandQueue const&) = delete;

        void push(EntityCommand command, void* data = nullptr) noexcept;
        std::optional<EntityCommand> pop() noexcept;
        bool loadComponent(EntityCommand command, void* dest) noexcept;
        size_t size() const noexcept;
        size_t poolCount() const noexcept;
        bool empty() const noexcept;
        void reset() noexcept;

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif