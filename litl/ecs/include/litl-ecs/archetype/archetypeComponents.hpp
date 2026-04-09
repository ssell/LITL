#ifndef LITL_ECS_ARCHETYPE_COMPONENT_ARRAY_H__
#define LITL_ECS_ARCHETYPE_COMPONENT_ARRAY_H__

#include <array>
#include <span>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/component/componentData.hpp"

namespace litl
{
    struct ChunkLayout;

    class ArchetypeComponents
    {
    public:

        ArchetypeComponents();
        ArchetypeComponents(std::span<ComponentTypeId> components);

        ComponentTypeId const& operator[](size_t index) const noexcept;

        void populate(ChunkLayout const* layout) noexcept;
        bool add(ComponentTypeId component) noexcept;
        bool add(std::span<ComponentTypeId> components) noexcept;
        bool add(std::span<ComponentData> components) noexcept;
        bool remove(ComponentTypeId component) noexcept;
        bool remove(std::span<ComponentTypeId> components) noexcept;
        bool has(ComponentTypeId component) const noexcept;

        size_t size() const noexcept;
        size_t capacity() const noexcept;
        uint64_t hash() noexcept;
        bool dirty() const noexcept;

    protected:

    private:

        std::array<ComponentTypeId, ecs::Constants::max_components> m_components{};
        size_t m_size{ 0 };
        uint64_t m_hash{ 0 };
        bool m_hashDirty{ false };
    };

    template<ValidComponentType ComponentType>
    void foldComponentTypesIntoArchetype(ArchetypeComponents& components)
    {
        components.add(ComponentDescriptor::get<ComponentType>()->id);
    }
}

#endif