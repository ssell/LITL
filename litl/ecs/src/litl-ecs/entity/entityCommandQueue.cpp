#include <cassert>
#include <queue>

#include "litl-core/constants.hpp"
#include "litl-ecs/entity/entityCommandQueue.hpp"
#include "litl-ecs/constants.hpp"
#include "litl-ecs/world.hpp"

namespace litl
{
    class EntityComponentPool
    {
    public:

        EntityComponentPool()
            : m_currOffset(0)
        {

        }

        bool insert(ComponentDescriptor const* descriptor, void* source, void** destination)
        {
            assert(descriptor != nullptr);

            if ((m_currOffset + descriptor->size) > ecs::Constants::entity_command_pool_size)
            {
                // pool if full
                return false;
            }

            (*destination) = get(m_currOffset);
            descriptor->move(source, (*destination));
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

        alignas(Constants::cache_line_size) std::byte m_buffer[ecs::Constants::entity_command_pool_size];
        uint32_t m_currOffset{ 0 };
    };

    struct EntityCommandQueue::Impl
    {
        std::vector<EntityCommand> entityCommands;
        std::vector<DeferredEntityCommand> deferredEntityCommands;
        std::vector<std::unique_ptr<EntityComponentPool>> componentPools;

        uint32_t currPool{ 0 };
        uint32_t currCommand{ 0 };
        uint32_t currDeferredCommand{ 0 };
        uint32_t createCommandCount{ 0 };

        void reset()
        {
            for (auto& pool : componentPools)
            {
                pool->reset();
            }

            currPool = 0;
            currCommand = 0;
            currDeferredCommand = 0;
            createCommandCount = 0;

            // command queue should already be empty, but just incase ...
            entityCommands.clear();
            deferredEntityCommands.clear();
        }

        void insertComponent(ComponentDescriptor const* descriptor, void* source, void** destination)
        {
            assert(descriptor != nullptr);

            if (!componentPools[currPool]->insert(descriptor, source, destination))
            {
                currPool++;

                if (currPool >= componentPools.size())
                {
                    componentPools.push_back(std::make_unique<EntityComponentPool>());
                }

                componentPools[currPool]->insert(descriptor, source, destination);
            }
        }
    };

    EntityCommandQueue::EntityCommandQueue()
        : m_pImpl(std::make_unique<EntityCommandQueue::Impl>())
    {
        m_pImpl->componentPools.push_back(std::make_unique<EntityComponentPool>());
        m_pImpl->entityCommands.reserve(512);
        m_pImpl->deferredEntityCommands.reserve(512);
    }

    EntityCommandQueue::~EntityCommandQueue()
    {

    }

    void EntityCommandQueue::push(EntityCommand command, void* source) noexcept
    {
        assert(command.type != EntityCommandType::CreateEntity);

        if ((command.type == EntityCommandType::AddComponent) && (source != nullptr))
        {
            m_pImpl->insertComponent(ComponentDescriptor::get(command.componentInfo.component), source, &command.componentInfo.data);
            command.queue = this;
        }

        m_pImpl->entityCommands.push_back(command);
    }

    void EntityCommandQueue::push(DeferredEntityCommand command, void* source) noexcept
    {
        if (command.type == EntityCommandType::CreateEntity)
        {
            m_pImpl->createCommandCount++;
        }
        else if ((command.type == EntityCommandType::AddComponent) && (source != nullptr))
        {
            m_pImpl->insertComponent(ComponentDescriptor::get(command.componentInfo.component), source, &command.componentInfo.data);
        }

        m_pImpl->deferredEntityCommands.push_back(command);
    }

    std::optional<EntityCommand> EntityCommandQueue::next() noexcept
    {
        if (empty())
        {
            return std::nullopt;
        }

        // We dont do a pop here or remove commands. Want to avoid the costly moves associated with that.
        // Commands are only removed via a reset which removes them all at once.
        return m_pImpl->entityCommands[m_pImpl->currCommand++];
    }

    bool EntityCommandQueue::loadComponent(EntityCommand command, void* dest) noexcept
    {
        if (command.type != EntityCommandType::AddComponent)
        {
            return false;
        }

        auto* descriptor = ComponentDescriptor::get(command.componentInfo.component);

        assert(descriptor != nullptr);
        assert(dest != nullptr);
        assert(command.componentInfo.data != nullptr);

        descriptor->move(command.componentInfo.data, dest);

        return true;
    }

    size_t EntityCommandQueue::count() const noexcept
    {
        return m_pImpl->entityCommands.size() - m_pImpl->currCommand;
    }

    size_t EntityCommandQueue::actionableCommandCount() const noexcept
    {
        return (m_pImpl->entityCommands.size() + m_pImpl->deferredEntityCommands.size()) - m_pImpl->createCommandCount;
    }

    size_t EntityCommandQueue::poolCount() const noexcept
    {
        return m_pImpl->componentPools.size();
    }

    bool EntityCommandQueue::empty() const noexcept
    {
        return count() == 0;
    }

    void EntityCommandQueue::reset() noexcept
    {
        m_pImpl->reset();
    }

    std::vector<DeferredEntityCommand> const& EntityCommandQueue::deferredCommands() const noexcept
    {
        return m_pImpl->deferredEntityCommands;
    }
}