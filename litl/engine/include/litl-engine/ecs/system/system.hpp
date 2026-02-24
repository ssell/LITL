#ifndef LITL_ENGINE_ECS_SYSTEM_H__
#define LITL_ENGINE_ECS_SYSTEM_H__

#include <concepts>
#include <tuple>

#include "litl-engine/ecs/constants.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"
#include "litl-engine/ecs/system/system.hpp"

namespace LITL::Engine::ECS
{
    template<typename System>
    concept ValidSystem = requires { typename System::SystemComponents; };

    /// <summary>
    /// Used to define the components that a System operates on.
    /// </summary>
    /// <typeparam name="...SystemComponents"></typeparam>
    template<typename... SystemComponents>
    struct SystemComponentList 
    {
    
    };

    /// <summary>
    /// Traits adapter used to extract the components.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    template<ValidSystem System>
    struct SystemTraits
    {
        using SystemComponents = typename System::SystemComponents;
    };

    template<typename ComponentList>
    struct ExpandSystemComponentList;

    /// <summary>
    /// Applies an operation to each component type in the system component list.
    /// </summary>
    /// <typeparam name="...Components"></typeparam>
    template<typename... Components>
    struct ExpandSystemComponentList<SystemComponentList<Components...>>
    {
        template<typename Fn>
        static void apply(Fn&& fn)
        {
            fn.template operator()<Components...>();
        }
    };

    /// <summary>
    /// Responsible for running a system over a single archetype chunk.
    /// The chunk is expected to have all of the components required by the system.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    template<ValidSystem System>
    class SystemRunner
    {
    public:

        SystemRunner(System& system, Archetype& archetype, Chunk& chunk)
            : m_refSystem(system), m_refArchetype(archetype), m_refChunk(chunk)
        {

        }

        /// <summary>
        /// () operator which is used by the ExpandSystemComponentList::apply utility.
        /// </summary>
        /// <typeparam name="...ComponentType"></typeparam>
        template<ValidComponentType... ComponentType>
        void operator()()
        {
            execute<ComponentType...>();
        }

    protected:

    private:

        System& m_refSystem;
        Archetype& m_refArchetype;
        Chunk& m_refChunk;

        /// <summary>
        /// Executes the system for the single Chunk.
        /// </summary>
        /// <typeparam name="...ComponentType"></typeparam>
        template<ValidComponentType... ComponentType>
        void execute()
        {
            // Retrieve the raw arrays for our component types
            auto componentArrays = std::tuple<ComponentType*...>
            {
                m_refChunk.getRawComponentArray<ComponentType>(m_refArchetype.chunkLayout()) ...
            };

            const uint32_t entityCount = m_refChunk.size();

            // Call System::update for each entity in the chunk.
            for (uint32_t i = 0; i < entityCount; ++i)
            {
                update(componentArrays, i, std::index_sequence_for<ComponentType...>{});
            }
        }

        /// <summary>
        /// Invokes the System::update method passing in the requested components.
        /// </summary>
        /// <typeparam name="ComponentTuple"></typeparam>
        /// <typeparam name="...Indices"></typeparam>
        /// <param name="componentArrays"></param>
        /// <param name="index"></param>
        /// <param name=""></param>
        template<typename ComponentTuple, size_t... Indices>
        void update(ComponentTuple& componentArrays, uint32_t const index, std::index_sequence<Indices...>)
        {
            m_refSystem.update(std::get<Indices>(componentArrays)[index] ...);
        }
    };
}

#endif