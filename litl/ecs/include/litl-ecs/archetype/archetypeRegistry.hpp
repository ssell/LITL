#ifndef LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__
#define LITL_ENGINE_ECS_ARCHETYPE_REGISTRY_H__

#include <initializer_list>
#include <span>
#include <vector>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/entity/entityRecord.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeComponents.hpp"

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

        /*
        template<ValidComponentType... ComponentTypes>
        static std::vector<ComponentTypeId> getComponentMask() noexcept
        {
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);
            refineComponentMask(componentTypeIds);

            return componentTypeIds;
        }
        */

        /// <summary>
        /// Retrieves (or creates) the archetype matching the specified component set.
        /// </summary>
        /// <typeparam name="...ComponentTypes"></typeparam>
        /// <returns></returns>
        template<ValidComponentType... ComponentTypes>
        static Archetype* get() noexcept
        {
            /*
            std::vector<ComponentTypeId> componentTypeIds;
            componentTypeIds.reserve(sizeof...(ComponentTypes));
            (foldComponentTypesIntoVector<ComponentTypes>(componentTypeIds), ...);

            return getByComponents(componentTypeIds);
            */

            ArchetypeComponents components;
            (foldComponentTypesIntoArchetype<ComponentTypes>(components), ...);
            return getByComponents(components);
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
        static Archetype* getByComponents(ArchetypeComponents& components) noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="components"></param>
        /// <returns></returns>
        static Archetype* getByComponents(std::initializer_list<ComponentTypeId> components) noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="record"></param>
        /// <param name="from"></param>
        /// <param name="to"></param>
        static void move(EntityRecord const& record, Archetype* from, Archetype* to) noexcept;

        /// <summary>
        /// Returns the number of Archetypes tracked by the registry.
        /// </summary>
        /// <returns></returns>
        static size_t archetypeCount() noexcept;

        /// <summary>
        /// Fetches all new Archetypes since the last fetch.
        /// </summary>
        /// <param name="clear"></param>
        /// <returns></returns>
        static std::vector<ArchetypeId> fetchNewArchetypes() noexcept;

    private:

        static void refineComponentMask(std::vector<ComponentTypeId>& componentTypeIds) noexcept;
        static Archetype* buildArchetype(uint64_t const archetypeHash, ArchetypeComponents const& components) noexcept;
    };
}

#endif