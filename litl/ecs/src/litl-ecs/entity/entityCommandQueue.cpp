#include <cassert>
#include <queue>
#include <vector>

#include "litl-ecs/entity/entityCommandQueue.hpp"
#include "litl-core/alignment.hpp"
#include "litl-ecs/constants.hpp"

namespace LITL::ECS
{
    class EntityComponentPool
    {
    public:

        EntityComponentPool()
            : m_currOffset(0)
        {

        }

        bool insert(ComponentDescriptor const* descriptor, void* source, uint32_t& destOffset)
        {
            assert(descriptor != nullptr);

            if ((m_currOffset + descriptor->size) > Constants::entity_command_pool_size)
            {
                // pool if full
                return false;
            }

            descriptor->move(source, get(m_currOffset));
            destOffset = m_currOffset;
            m_currOffset += descriptor->size;

            return true;
        }

        uint32_t offset() const noexcept
        {
            return m_currOffset;
        }

        void reset() noexcept
        {
            m_currOffset = 0;
        }

        [[nodiscard]] std::byte* get(uint32_t offset)
        {
            return m_buffer + offset;
        }

    private:

        alignas(CacheLineSize) std::byte m_buffer[Constants::entity_command_pool_size];
        uint32_t m_currOffset{ 0 };
    };

    struct EntityCommandQueue::Impl
    {
        std::queue<EntityCommand> m_queue;
        std::vector<std::unique_ptr<EntityComponentPool>> m_componentPools;
        uint32_t m_currPool;

        void reset()
        {
            for (auto& pool : m_componentPools)
            {
                pool->reset();
            }

            m_currPool = 0;

            // command queue should already be empty, but just incase ...
            while (!m_queue.empty()) { m_queue.pop(); }
        }
    };

    EntityCommandQueue::EntityCommandQueue()
        : m_pImpl(std::make_unique<EntityCommandQueue::Impl>())
    {
        m_pImpl->m_componentPools.push_back(std::make_unique<EntityComponentPool>());
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

            // Can only fail if the pool is out of room.
            if (!m_pImpl->m_componentPools[m_pImpl->m_currPool]->insert(descriptor, data, command.offset))
            {
                m_pImpl->m_currPool++;

                if (m_pImpl->m_currPool >= m_pImpl->m_componentPools.size())
                {
                    m_pImpl->m_componentPools.push_back(std::make_unique<EntityComponentPool>());
                }

                m_pImpl->m_componentPools[m_pImpl->m_currPool]->insert(descriptor, data, command.offset);
            }

            command.pool = m_pImpl->m_currPool;

            m_pImpl->m_queue.push(command);
        }
    }

    std::optional<EntityCommand> EntityCommandQueue::pop() noexcept
    {
        if (empty())
        {
            return std::nullopt;
        }

        auto ret = m_pImpl->m_queue.front(); m_pImpl->m_queue.pop();
        return ret;
    }

    bool EntityCommandQueue::loadComponent(EntityCommand command, void* dest) noexcept
    {
        if (command.type != EntityCommandType::AddComponent)
        {
            return false;
        }

        auto* descriptor = ComponentDescriptor::get(command.component);

        assert(descriptor != nullptr);
        assert(dest != nullptr);
        assert(command.pool < m_pImpl->m_componentPools.size());
        assert(command.offset < Constants::entity_command_pool_size);

        descriptor->move(m_pImpl->m_componentPools[command.pool]->get(command.offset), dest);

        return true;
    }

    size_t EntityCommandQueue::size() const noexcept
    {
        return m_pImpl->m_queue.size();
    }

    size_t EntityCommandQueue::poolCount() const noexcept
    {
        return m_pImpl->m_componentPools.size();
    }

    bool EntityCommandQueue::empty() const noexcept
    {
        return m_pImpl->m_queue.empty();
    }

    void EntityCommandQueue::reset() noexcept
    {
        m_pImpl->reset();
    }
}