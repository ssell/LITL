#include <vector>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/archetype/archetype.hpp"

namespace LITL::ECS
{
    struct System::Impl
    {
        const SystemTypeId id;
        std::vector<Archetype*> archetypes;
    };

    namespace
    {
        static SystemTypeId nextSystemId()
        {
            static SystemTypeId id = 0;
            return id++;
        }
    }

    System::System()
        : m_pImpl(std::make_unique<System::Impl>(nextSystemId()))
    {

    }
    
    System::~System()
    {

    }

    SystemTypeId System::id() const noexcept
    {
        return m_pImpl->id;
    }

    void System::updateArchetypes(std::vector<ArchetypeId> const& newArchetypes) const noexcept
    {
        // ... todo ... go through each new chunk and any that match our inner system requirements are added to m_pImpl->archetypes ...
    }

    void System::run(World& world, float dt)
    {
        assert(m_runFunc != nullptr);

        // ... todo multithread this ... each chunk should be its own job/task ...

        for (auto archetype : m_pImpl->archetypes)
        {
            auto chunkCount = archetype->chunkCount();
            auto const& layout = archetype->chunkLayout();

            for (auto ci = 0; ci < chunkCount; ++ci)
            {
                m_runFunc(world, dt, archetype->getChunk(ci), layout);
            }
        }
    }
}