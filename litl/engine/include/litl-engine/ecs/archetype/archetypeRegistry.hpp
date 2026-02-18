#ifndef LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__
#define LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__

#include <memory>
#include <span>
#include <type_traits>
#include <vector>

#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Responsible for owning and tracking all Archetype specializations.
    /// </summary>
    class ArchetypeRegistry
    {
    public:

        ArchetypeRegistry();
        ArchetypeRegistry(ArchetypeRegistry const&) = delete;
        ArchetypeRegistry& operator=(ArchetypeRegistry const&) = delete;

        template<typename... ComponentTypes>
        Archetype const* get()
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (get_addComponent<ComponentTypes>(&componentTypeIds), ...);

            return get_internal(componentTypeIds);
        }

        template<typename... ComponentTypeIds>
        Archetype const* get(ComponentTypeIds... componentTypes)
        {
            static_assert((std::is_convertible_v<ComponentTypeIds, ComponentTypeId> && ...));
            std::vector<ComponentTypeId> componentTypeIds{ static_cast<ComponentTypeId>(componentTypes)... };
            return get_internal(componentTypeIds);
        }

    protected:

    private:

        template<typename ComponentType>
        static void get_addComponent(std::vector<ComponentTypeId>& componentTypeIds)
        {
            componentTypeIds.emplace_back(ComponentDescriptor::get<ComponentType>()->id);
        }

        Archetype const* get_internal(std::vector<ComponentTypeId>& components);

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif