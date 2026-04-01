#include <algorithm>
#include <cassert>

#include "litl-core/hash.hpp"
#include "litl-core/containers/fixedSortedArray.hpp"
#include "litl-ecs/archetype/archetypeComponents.hpp"
#include "litl-ecs/archetype/chunkLayout.hpp"

namespace LITL::ECS
{
    ArchetypeComponents::ArchetypeComponents()
    {

    }

    ArchetypeComponents::ArchetypeComponents(std::span<ComponentTypeId> components)
    {
        assert(components.size() <= Constants::max_components);

        for (auto i = 0; i < components.size(); ++i)
        {
            m_components[i] = components[i];
        }

        m_size = components.size();
        m_hashDirty = true;
    }

    ComponentTypeId const& ArchetypeComponents::operator[](size_t index) const noexcept
    {
        assert(index < m_size);
        return m_components[index];
    }

    void ArchetypeComponents::populate(ChunkLayout const* layout) noexcept
    {
        assert(layout->componentTypeCount < Constants::max_components);

        for (auto i = 0; i < layout->componentTypeCount; ++i)
        {
            m_components[i] = layout->componentOrder[i]->id;
        }

        m_size = layout->componentTypeCount;
        m_hashDirty = true;
        hash();
    }

    bool ArchetypeComponents::add(ComponentTypeId component) noexcept
    {
        if (m_size == Constants::max_components)
        {
            return false;
        }

        m_components[m_size++] = component;
        m_hashDirty = true;

        return true;
    }

    bool ArchetypeComponents::add(std::span<ComponentTypeId> components) noexcept
    {
        if ((m_size + components.size()) > Constants::max_components)
        {
            return false;
        }

        for (auto i = 0; i < components.size(); ++i)
        {
            m_components[m_size + i] = components[i];
        }

        m_size += components.size();
        m_hashDirty = true;

        return true;
    }

    bool ArchetypeComponents::add(std::span<ComponentData> components) noexcept
    {
        if ((m_size + components.size()) > Constants::max_components)
        {
            return false;
        }

        for (auto i = 0; i < components.size(); ++i)
        {
            m_components[m_size + i] = components[i];
        }

        m_size += components.size();
        m_hashDirty = true;

        return true;
    }

    bool ArchetypeComponents::remove(ComponentTypeId component) noexcept
    {
        bool anyRemoved = false;

        for (auto i = 0; i < m_size; ++i)
        {
            if (m_components[i] == component)
            {
                m_components[i] = m_components[m_size - 1];
                m_size--;
                anyRemoved = true;
            }
        }

        m_hashDirty = m_hashDirty || anyRemoved;

        return anyRemoved;
    }

    bool ArchetypeComponents::remove(std::span<ComponentTypeId> components) noexcept
    {
        bool anyRemoved = false;

        for (auto& component : components)
        {
            if (remove(component))
            {
                anyRemoved = true;
            }
        }

        return anyRemoved;
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

        std::sort(m_components.begin(), m_components.begin() + m_size);
        auto iter = std::unique(m_components.begin(), m_components.begin() + m_size);
        m_size = static_cast<std::size_t>(iter - m_components.begin());

        m_hash = (m_size == 0 ? 0 : Core::hashSubarray<ComponentTypeId>(m_components, 0, m_size));
        m_hashDirty = false;

        return m_hash;
    }

    bool ArchetypeComponents::dirty() const noexcept
    {
        return m_hashDirty;
    }
}