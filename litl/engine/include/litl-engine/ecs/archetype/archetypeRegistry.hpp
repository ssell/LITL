#ifndef LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__
#define LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__

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
    private:

        template<typename ComponentType>
        static void addComponentFold(std::vector<ComponentTypeId>& componentTypeIds)
        {
            componentTypeIds.emplace_back(ComponentDescriptor::get<ComponentType>()->id);
        }

        static Archetype const* get_internal(std::vector<ComponentTypeId>& components);

    public:

        /// <summary>
        /// Retrieves (or creates) the archetype matching the specified component set.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <returns></returns>
        template<typename... ComponentTypes>
        static Archetype const* get()
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (addComponentFold<ComponentTypes>(componentTypeIds), ...);

            return get_internal(componentTypeIds);
        }

        /// <summary>
        /// Retrieves (or creates) the archetype matching the specified component set.
        /// </summary>
        /// <typeparam name="...ComponentTypeIds"></typeparam>
        /// <param name="...componentTypes"></param>
        /// <returns></returns>
        template<typename... ComponentTypeIds>
        static Archetype const* get(ComponentTypeIds... componentTypes)
        {
            static_assert((std::is_convertible_v<ComponentTypeIds, ComponentTypeId> && ...));
            std::vector<ComponentTypeId> componentTypeIds{ static_cast<ComponentTypeId>(componentTypes)... };
            return get_internal(componentTypeIds);
        }
    };
}

#endif