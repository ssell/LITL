#ifndef LITL_ECS_ARCHETYPE_COMPONENT_ARRAY_H__
#define LITL_ECS_ARCHETYPE_COMPONENT_ARRAY_H__

#include <array>
#include <span>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/component/component.hpp"

namespace LITL::ECS
{
    struct ChunkLayout;

    class ArchetypeComponents
    {
    public:

        ArchetypeComponents();
        ArchetypeComponents(std::span<ComponentTypeId> components);

        ComponentTypeId& operator[](size_t index) noexcept;
        ComponentTypeId const& operator[](size_t index) const noexcept;

        void populate(ChunkLayout const* layout) noexcept;
        bool add(ComponentTypeId component) noexcept;
        bool add(std::span<ComponentTypeId> components) noexcept;
        bool remove(ComponentTypeId component) noexcept;
        bool remove(std::span<ComponentTypeId> components) noexcept;

        size_t size() const noexcept;
        size_t capacity() const noexcept;
        uint64_t hash() noexcept;

    protected:

    private:

        std::array<ComponentTypeId, Constants::max_components> m_components{};
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