#include <cassert>

#include "litl-ecs/archetype/archetypeComponents.hpp"
#include "litl-ecs/archetype/chunkLayout.hpp"

namespace LITL::ECS
{
    ArchetypeComponents::ArchetypeComponents()
    {

    }

    void ArchetypeComponents::populate(ChunkLayout const* layout) noexcept
    {
        assert(layout->componentTypeCount < Constants::max_components);

        for (auto i = 0; i < layout->componentTypeCount; ++i)
        {
            m_components[i] = layout->componentOrder[i]->id;
        }
    }

    bool ArchetypeComponents::add(ComponentTypeId component) noexcept
    {
        if (m_size == Constants::max_components)
        {
            return false;
        }

        m_components[m_size++] = component;

        return true;
    }

    bool ArchetypeComponents::add(std::span<ComponentTypeId> components) noexcept
    {
        if ((m_size + components.size()) > Constants::max_components)
        {
            return false;
        }

        // ... todo ...

        return true;
    }

    bool ArchetypeComponents::remove(ComponentTypeId component) noexcept
    {
        return false;
    }

    bool ArchetypeComponents::remove(std::span<ComponentTypeId> components) noexcept
    {
        /*
        
        desiredComponents.erase(
            std::remove_if(
                desiredComponents.begin(),
                desiredComponents.end(),
                [&](ComponentTypeId componentType) { return std::find(components.begin(), components.end(), componentType) != components.end(); }
            ),
            desiredComponents.end());
        
        */
        return false;
    }

    size_t ArchetypeComponents::size() const noexcept
    {
        return m_size;
    }

    size_t ArchetypeComponents::capacity() const noexcept
    {
        return Constants::max_components;
    }

    uint64_t ArchetypeComponents::hash() noexcept
    {
        if (!m_hashDirty)
        {
            return m_hash;
        }

        // ... todo hash ...

        m_hashDirty = false;
        return m_hash;
    }
}