#include <cassert>
#include <queue>
#include <vector>

#include "litl-ecs/entity/entityCommandQueue.hpp"
#include "litl-core/alignment.hpp"

namespace LITL::ECS
{
    constexpr uint32_t EntityComponentPoolBufferSize = 1024 * 16;       // 16kb pool

    class EntityComponentPool
    {
    public:

        bool insert(ComponentDescriptor const* descriptor, void* source)
        {
            assert(descriptor != nullptr);

            if (!fits(descriptor->size))
            {
                // pool if full
                return false;
            }

            descriptor->move(source, get(m_currOffset));
            m_currOffset += descriptor->size;

            return true;
        }

        bool fits(uint32_t size) const noexcept
        {
            return (m_currOffset + size) < EntityComponentPoolBufferSize;
        }

        void reset() noexcept
        {
            m_currOffset = 0;
        }

        [[nodiscard]] void* get(uint32_t offset)
        {
            return m_buffer + offset;
        }

    private:

        alignas(CacheLineSize) std::byte m_buffer[EntityComponentPoolBufferSize];
        uint32_t m_currOffset;
    };

    struct EntityCommandQueue::Impl
    {
        std::queue<EntityCommand> m_queue;
        std::vector<std::unique_ptr<EntityComponentPool>> m_componentPools;
        uint32_t m_currPool;

        bool reset()
        {
            for (auto& pool : m_componentPools)
            {
                pool.reset();
            }

            m_currPool = 0;
        }
    };

    EntityCommandQueue::EntityCommandQueue()
        : m_pImpl(std::make_unique<EntityCommandQueue::Impl>())
    {
        m_pImpl->m_componentPools.emplace_back();
    }

    EntityCommandQueue::~EntityCommandQueue()
    {

    }

    void EntityCommandQueue::push(EntityCommand command, void* data) noexcept
    {
        if ((command.type != EntityCommandType::AddComponent) || (data == nullptr))
        {
            m_pImpl->m_queue.push(command);
        }
        else
        {
            // Have component data that needs to be stored.
            auto* descriptor = ComponentDescriptor::get(command.component);
            assert(descriptor != nullptr);

            if (!m_pImpl->m_componentPools[m_pImpl->m_currPool]->insert(descriptor, data))
            {
                // Can only fail if the pool is out of room.
                m_pImpl->m_componentPools.emplace_back();
                m_pImpl->m_currPool++;

                m_pImpl->m_componentPools[m_pImpl->m_currPool]->insert(descriptor, data);
            }
        }
    }

    std::optional<EntityCommand> EntityCommandQueue::pop() noexcept
    {
        return std::nullopt;
    }

    size_t EntityCommandQueue::size() const noexcept
    {
        return m_pImpl->m_queue.size();
    }

    bool EntityCommandQueue::empty() const noexcept
    {
        return m_pImpl->m_queue.empty();
    }
}