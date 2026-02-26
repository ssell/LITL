#ifndef LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__
#define LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__

#include <initializer_list>
#include <span>
#include <vector>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/entityRecord.hpp"
#include "litl-ecs/archetype/archetype.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// Responsible for owning and tracking all Archetype specializations.
    /// Each component in an archetype definition is unique and the set is ordered based on component id.
    /// For example: Archetype<Foo, Bar> == Archetype<Bar, Bar, Foo, Foo, Foo>
    /// </summary>
    class ArchetypeRegistry
    {
    public:

        /// <summary>
        /// Returns the empty archetype.
        /// </summary>
        /// <returns></returns>
        static Archetype* Empty() noexcept;

        template<ValidComponentType... ComponentTypes>
        static std::vector<ComponentTypeId> getComponentMask() noexcept
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);
            refineComponentMask(componentTypeIds);

            return componentTypeIds;
        }

        /// <summary>
        /// Retrieves (or creates) the archetype matching the specified component set.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <returns></returns>
        template<ValidComponentType... ComponentTypes>
        static Archetype* get() noexcept
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);

            return getByComponents(componentTypeIds);
        }

        /// <summary>
        /// Retrieves the archetype by the internal registry index.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        static Archetype* getById(ArchetypeId id) noexcept;

        /// <summary>
        /// Retrieves the archetype by the component hash.
        /// </summary>
        /// <param name="componentHash"></param>
        /// <returns></returns>
        static Archetype* getByComponentHash(uint64_t componentHash) noexcept;

        /// <summary>
        /// Retrieves the archetype by the provided list of component ids.
        /// </summary>
        /// <param name="components"></param>
        /// <returns></returns>
        static Archetype* getByComponents(std::vector<ComponentTypeId> componentTypeIds) noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="record"></param>
        /// <param name="from"></param>
        /// <param name="to"></param>
        static void move(EntityRecord const& record, Archetype* from, Archetype* to) noexcept;

    private:

        static void refineComponentMask(std::vector<ComponentTypeId>& componentTypeIds) noexcept;
        static Archetype* buildArchetype(uint64_t archetypeHash, std::vector<ComponentTypeId> const& componentTypeIds) noexcept;
    };
}

#endif