#include "litl-ecs/entity/entityCommandQueue.hpp"

namespace LITL::ECS
{
    struct EntityCommandQueue::Impl
    {

    };

    EntityCommandQueue::EntityCommandQueue()
        : m_pImpl(std::make_unique<EntityCommandQueue::Impl>())
    {

    }

    EntityCommandQueue::~EntityCommandQueue()
    {

    }

    void EntityCommandQueue::push(EntityCommand command, void* data) noexcept
    {

    }

    std::optional<EntityCommand> EntityCommandQueue::pop() noexcept
    {
        return std::nullopt;
    }

    size_t EntityCommandQueue::size() const noexcept
    {
        return 0;
    }

    bool EntityCommandQueue::empty() const noexcept
    {
        return size() == 0;
    }
}