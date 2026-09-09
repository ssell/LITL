#ifndef LITL_ECS_SYSTEM_COMPONENT_OPERATIONS_H__
#define LITL_ECS_SYSTEM_COMPONENT_OPERATIONS_H__

#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "litl-ecs/system/systemTraits.hpp"
#include "litl-ecs/archetype/chunk.hpp"

namespace litl
{
    template<typename SystemComponentsTuple>
    struct SystemComponentOperations;

    /// <summary>
    /// Collection of static operations that are run over component sets.
    /// </summary>
    template<typename... ComponentTypes>
    struct SystemComponentOperations<std::tuple<ComponentTypes...>>
    {
        /// <summary>
        /// Returns a vector of the plain component types required by the system ::update method.
        /// For example `update(SystemData const&, Entity, Foo&, Bar const&)` would return `[Foo, Bar]`
        /// </summary>
        static std::vector<ComponentTypeId> extractRequiredComponentIds()
        {
            std::vector<ComponentTypeId> ids;
            ids.reserve(sizeof...(ComponentTypes));

            ((SystemParamTraits<ComponentTypes>::arity == ComponentArity::Required ?
                void(ids.push_back(getComponentTypeId<typename SystemParamTraits<ComponentTypes>::ComponentType>())) :
                void()), ...);

            return ids;
        }

        /// <summary>
        /// Returns a vector of the plain component types excluded by the system ::update method
        /// and that can not be present in any archetype for it to be valid for iteration.
        /// </summary>
        static std::vector<ComponentTypeId> extractExcludedComponentIds()
        {
            std::vector<ComponentTypeId> ids;
            ids.reserve(sizeof...(ComponentTypes));

            (
                (SystemParamTraits<ComponentTypes>::arity == ComponentArity::Excluded ?
                    void(ids.push_back(getComponentTypeId<typename SystemParamTraits<ComponentTypes>::ComponentType>())) :
                    void()),
                ...);

            return ids;
        }

        template<typename Param>
        static void appendComponentInfo(std::vector<SystemComponentInfo>& infos)
        {
            // Excluded components are never read or written, so they must not produce a scheduling edge.
            if constexpr (SystemParamTraits<Param>::arity != ComponentArity::Excluded)
            {
                infos.push_back(SystemComponentInfo{
                    .id = getComponentTypeId<typename SystemParamTraits<Param>::ComponentType>(),
                    .readonly = SystemParamTraits<Param>::access == ComponentAccessType::Read
                    });
            }

            // ^ note the above use of `if constexpr` instead of the ternary in extractExcludedComponentIds.
            // In extract..., the use of getComponentTypeId and ::arity on all valid parameters.
            // However here we make use of ::access which is not present on Without. With the ternary, both branch
            // operands are instantiated regardless of the outcome. So a ternary there would be an error. 
            // But with `if constexpr` the untaken branch is completely discarded and so Without is safe.
        }

        static std::vector<SystemComponentInfo> extractComponentInfo()
        {
            std::vector<SystemComponentInfo> componentInfos;
            componentInfos.reserve(sizeof...(ComponentTypes));

            (appendComponentInfo<ComponentTypes>(componentInfos), ...);

            return componentInfos;
        }

        template<typename Param>
        static auto extractComponentBuffer(Chunk& chunk, ChunkLayout const& layout) -> typename SystemParamTraits<Param>::ComponentType*
        {
            using traits = SystemParamTraits<Param>;
            using ComponentType = typename traits::ComponentType;

            if constexpr (traits::arity == ComponentArity::Required)
            {
                // asserts if absent
                return chunk.getRawComponentArray<ComponentType>(layout);
            }
            else if constexpr (traits::arity == ComponentArity::Optional)
            {
                // nullptr if absent
                return chunk.tryGetRawComponentArray<ComponentType>(layout);
            }
            else
            {
                // excluded: matching guarantees the archetype lacks this column
                return nullptr;
            }
        }

        static auto extractComponentBuffers(Chunk& chunk, ChunkLayout const& layout)
        {
            return std::tuple
            {
                extractComponentBuffer<ComponentTypes>(chunk, layout)...
            };
        }

        /// <summary>
        /// Invokes System::update once per entity in the chunk.
        /// </summary>
        template<ValidSystem S>
        static void forEach(S* system, SystemData const& data, Chunk& chunk, ChunkLayout const& layout)
        {
            forEachImpl(system, data, chunk, layout, std::index_sequence_for<ComponentTypes...>{});
        }

    private:

        /// <summary>
        /// Invokes System::update once per entity in the chunk.
        /// Indices runs in lockstep with ComponentTypes so that each parameter is bound by its own SystemParamTraits against its own column.
        /// </summary>
        template<ValidSystem S, std::size_t... Indices>
        static void forEachImpl(S* system, SystemData const& data, Chunk& chunk, ChunkLayout const& layout, std::index_sequence<Indices...>)
        {
            auto entities = chunk.getEntities(layout);

            if (entities.empty())
            {
                return;
            }

            auto columns = extractComponentBuffers(chunk, layout);

            for (uint32_t i = 0u; i < static_cast<uint32_t>(entities.size()); ++i)
            {
                system->update(data, entities[i], SystemParamTraits<ComponentTypes>::bind(std::get<Indices>(columns), i)...);
            }
        }
    };

    /// <summary>
    /// Shorthand utility to get all of the SystemComponentInfo for a valid system.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    /// <returns></returns>
    template<ValidSystem System>
    std::vector<SystemComponentInfo> ExtractSystemComponentInfo()
    {
        return SystemComponentOperations<SystemComponents<System>>::extractComponentInfo();
    }
}

#endif