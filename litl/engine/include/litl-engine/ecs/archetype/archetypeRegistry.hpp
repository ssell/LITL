#ifndef LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__
#define LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__

#include <initializer_list>
#include <span>
#include <vector>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entityRecord.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Responsible for owning and tracking all Archetype specializations.
    /// Each component in an archetype definition is unique and the set is ordered based on component id.
    /// For example: Archetype<Foo, Bar> == Archetype<Bar, Bar, Foo, Foo, Foo>
    /// </summary>
    class ArchetypeRegistry
    {
    private:

        template<typename ComponentType>
        static void fold(std::vector<ComponentTypeId>& componentTypeIds) noexcept
        {
            componentTypeIds.emplace_back(ComponentDescriptor::get<ComponentType>()->id);
        }

        static Archetype const* get_internal(std::vector<ComponentTypeId> componentTypeIds) noexcept;

    public:

        /// <summary>
        /// Retrieves (or creates) the archetype matching the specified component set.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <returns></returns>
        template<typename... ComponentTypes>
        static Archetype const* get() noexcept
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (fold<ComponentTypes>(componentTypeIds), ...);

            return get_internal(componentTypeIds);
        }

        /// <summary>
        /// Retrieves the archetype by the internal registry index.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        static Archetype const* getByIndex(uint32_t index) noexcept;

        /// <summary>
        /// Retrieves the archetype by the component hash.
        /// </summary>
        /// <param name="componentHash"></param>
        /// <returns></returns>
        static Archetype const* getByComponentHash(uint64_t componentHash) noexcept;

        /// <summary>
        /// Retrieves the archetype by the provided list of component ids.
        /// </summary>
        /// <param name="components"></param>
        /// <returns></returns>
        static Archetype const* getByComponents(std::initializer_list<ComponentTypeId> componentTypeIds) noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="record"></param>
        /// <param name="from"></param>
        /// <param name="to"></param>
        static void move(EntityRecord* record, Archetype* from, Archetype* to) noexcept;
    };
}

#endif