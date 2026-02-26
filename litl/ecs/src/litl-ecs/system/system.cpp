#include <vector>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

namespace LITL::ECS
{
    struct System::Impl
    {
        const SystemTypeId id;

        std::vector<ComponentTypeId> componentTypes;
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

    void System::registerComponentType(ComponentTypeId const componentType) const noexcept
    {
        m_pImpl->componentTypes.push_back(componentType);
    }


    void System::updateArchetypes(std::vector<ArchetypeId> const& newArchetypes) const noexcept
    {
        for (auto archetypeId : newArchetypes)
        {
            bool validArchetype = true;
            auto* archetype = ArchetypeRegistry::getById(archetypeId);
            
            for (auto i = 0; i < m_pImpl->componentTypes.size() && validArchetype; ++i)
            {
                validArchetype = validArchetype && archetype->hasComponent(m_pImpl->componentTypes[i]);
            }

            if (validArchetype)
            {
                m_pImpl->archetypes.push_back(archetype);
            }
        }
    }

    void System::run(World& world, float dt)
    {
        assert(m_runFunc != nullptr);

        // ... todo multithread this ... each chunk should be its own job/task ...

        for (auto archetype : m_pImpl->archetypes)
        {
            const auto chunkCount = archetype->chunkCount();
            const auto& layout = archetype->chunkLayout();

            for (auto ci = 0; ci < chunkCount; ++ci)
            {
                m_runFunc(world, dt, archetype->getChunk(ci), layout);
            }
        }
    }
}