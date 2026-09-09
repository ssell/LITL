#ifndef LITL_ECS_SYSTEM_TRAITS_H__
#define LITL_ECS_SYSTEM_TRAITS_H__

#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <vector>

#include "litl-core/traits.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/archetype/chunk.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/system/systemData.hpp"

namespace litl
{
    class EntityCommands;

    /// <summary>
    /// Wraps a component to flag that it should not be in any iterable archetypes.
    /// </summary>
    template<ValidComponentType C> struct Without {};

    template<typename> struct IsWithout : std::false_type {};
    template<typename C> struct IsWithout<Without<C>> : std::true_type {};

    /// <summary>
    /// A component type as it may appear inside a System::update parameter.
    /// Deliberately rejects cv-qualified C so that each parameter spelling has exactly one viable specialization below (no reliance on partial ordering).
    /// Additionally rejects any 'Without<C>' wrapper as Without itself is a valid component as per ValidComponentType constraints.
    /// </summary>
    template<typename C>
    concept SystemComponentType = 
        ValidComponentType<C> && 
        std::same_as<C, std::remove_cv_t<C>> &&
        !IsWithout<C>::value;

    enum class ComponentAccessType : uint8_t
    {
        Read = 0,
        Write = 1
    };

    enum class ComponentArity : uint8_t
    {
        Required = 0,
        Optional = 1,
        Excluded = 2
    };

    /// <summary>
    /// Primary template: not a valid system parameter. Specializations opt in.
    /// </summary>
    template<typename Param>
    struct SystemParamTraits
    {
        static constexpr bool valid = false;
    };

    /// <summary>
    /// A non-const reference parameter. For example `Foo& foo`.
    /// These are writable components required by the system.
    /// </summary>
    template<SystemComponentType C>
    struct SystemParamTraits<C&>
    {
        static constexpr bool valid = true;
        using ComponentType = C;
        static constexpr auto access = ComponentAccessType::Write;
        static constexpr auto arity = ComponentArity::Required;

        static C& bind(C* column, uint32_t index) noexcept
        {
            return column[index];
        }
    };

    /// <summary>
    /// A const reference parameter. For example `Foo const& foo`.
    /// These are read-only components required by the system.
    /// </summary>
    template<SystemComponentType C>
    struct SystemParamTraits<C const&>
    {
        static constexpr bool valid = true;
        using ComponentType = C;
        static constexpr auto access = ComponentAccessType::Read;
        static constexpr auto arity = ComponentArity::Required;

        static C const& bind(C* column, uint32_t index) noexcept
        {
            return column[index];
        }
    };

    /// <summary>
    /// A non-const pointer parameter. For example `Foo* foo`.
    /// These are writable components that are optional for the system to run.
    /// </summary>
    template<SystemComponentType C>
    struct SystemParamTraits<C*>
    {
        static constexpr bool valid = true;
        using ComponentType = C;
        static constexpr auto access = ComponentAccessType::Write;
        static constexpr auto arity = ComponentArity::Optional;

        static C* bind(C* column, uint32_t index) noexcept
        {
            return (column ? column + index : nullptr);
        }
    };

    /// <summary>
    /// A const pointer parameter. For example `Foo const* foo`.
    /// These are read-only components that are optional for the system to run.
    /// </summary>
    template<SystemComponentType C>
    struct SystemParamTraits<C const*>
    {
        static constexpr bool valid = true;
        using ComponentType = C;
        static constexpr auto access = ComponentAccessType::Read;
        static constexpr auto arity = ComponentArity::Optional;

        static C const* bind(C* column, uint32_t index) noexcept
        {
            return (column ? column + index : nullptr);
        }
    };

    /// <summary>
    /// Indicates that the system will not run over any archetypes that the specified component.
    /// </summary>
    template<SystemComponentType C>
    struct SystemParamTraits<Without<C>>
    {
        static constexpr bool valid = true;
        using ComponentType = C;
        static constexpr auto arity = ComponentArity::Excluded;
        // Exclusion of `access` is intentional. An excluded component is never read or written.

        static Without<C> bind(C*, uint32_t) noexcept 
        {
            return {};
        }
    };

    template<typename Param>
    consteval bool ValidSystemParam()
    {
        static_assert(SystemParamTraits<Param>::valid, "System::update component arguments must be T&, T const&, T*, T const*, or Without<T>.");
        return SystemParamTraits<Param>::valid;
    }

    /// <summary>
    /// The number of occurrences of C among the component types of Params...
    /// </summary>
    template<typename C, typename... Params>
    consteval std::size_t CountSystemComponent()
    {
        return (std::size_t{ 0 } + ... + (std::same_as<C, typename SystemParamTraits<Params>::ComponentType> ? 1u : 0u));
    }

    template<typename ComponentsTuple>
    struct SystemComponentsValidation;

    /// <summary>
    /// Validates that all individual parameters are valid and that there are no duplicates (based on type).
    /// </summary>
    template<typename... Params>
    struct SystemComponentsValidation<std::tuple<Params...>>
    {
        static consteval bool allParamsValid()
        {
            return (ValidSystemParam<Params>() && ...);
        }

        static consteval bool noDuplicateComponents()
        {
            return ((CountSystemComponent<typename SystemParamTraits<Params>::ComponentType, Params...>() == 1) && ...);
        }
    };

    /// <summary>
    /// The system update methods must begin with "EntityCommands&,float,Entity" and so those are not needed for
    /// custom system parameter decomposition. This removes the always-present mandator parameters.
    /// 
    /// For example: 
    /// 
    ///     update(SystemData const&, Entity entity, Foo& foo, Bar const& bar)
    /// 
    /// The following types are extracted:
    /// 
    ///     (Foo&, Bar const&)
    /// </summary>
    /// <typeparam name="Tuple"></typeparam>
    /// <typeparam name="...I"></typeparam>
    /// <param name=""></param>
    /// <returns></returns>
    template<typename Tuple, std::size_t... I>
    auto SystemTupleTailImpl(std::index_sequence<I...>) -> std::tuple<std::tuple_element_t<I + 2, Tuple>...>;
    //                                                                                    ^ extract [2, 3, 4, ...], skipping [0] (SystemData, Entity)

    /// <summary>
    /// Works with SystemTupleTailImpl to retrieve all but the first two (mandatory SystemData,Entity) types in the update signature.
    /// </summary>
    /// <typeparam name="Tuple"></typeparam>
    template<typename Tuple>
    using SystemTupleTail = decltype(SystemTupleTailImpl<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple> - 2>{})); 
    //                                                                                    ^ reduce size by 2 so we dont go OOB in the Impl

    /// <summary>
    /// Requirements for a valid System class/struct.
    /// 
    /// All that is needed is there is an "update" method that takes in a EntityCommands& and float parameter.
    /// Additional parameters can be added and are used for archetype matching and the values are 
    /// provided during system run/iteration.
    /// </summary>
    template<typename S>
    concept ValidSystem = requires(S s, ServiceProvider& services)
    {
        { s.setup(services) } -> std::same_as<void>;                            // must have a "setup(ServiceProvider& services)" method
        { s.prepare() } -> std::same_as<void>;                                  // must have a "prepare()" method
        &S::update;                                                             // must have an "update" method (more on that below)
    }
    && [] {
        using traits = MethodTraits<decltype(&S::update)>;
        using args = typename traits::argsTuple;

        static_assert(std::tuple_size_v<args> >= 2, "System::update must take atleast (SystemData const&, Entity)");

        if constexpr (std::tuple_size_v<args> >= 2)
        {
            using components = SystemTupleTail<args>;
            using validation = SystemComponentsValidation<components>;

            if constexpr (validation::allParamsValid())
            {
                static_assert(validation::noDuplicateComponents(), "Each component type may appear at most once in System::update (Without<T> included).");
            }

            using Arg0 = std::tuple_element_t<0, args>;
            using Arg1 = std::tuple_element_t<1, args>;

            static_assert(std::same_as<typename traits::returnType, void>, "System::update return type must be void.");
            static_assert(std::same_as<Arg0, SystemData const&>, "System::update first argument must be 'SystemData const&'");
            static_assert(std::same_as<Arg1, Entity>, "System::update second argument must be 'Entity'");
        }

        return true;
    } ();

    /// <summary>
    /// Retrieves the tuple of types required by the System::update method (excluding the mandatory EntityCommands&,float).
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<ValidSystem S>
    using SystemComponents = SystemTupleTail<typename MethodTraits<decltype(&S::update)>::argsTuple>;
    //                       ^ remove the first two arguments                            ^ extract the arguments


    /// <summary>
    /// Stores the component id (which can be used with ComponentDescriptor to fetch it)
    /// and if the component is being used by the system in a read-only or read-write manner.
    /// </summary>
    struct SystemComponentInfo
    {
        ComponentTypeId id{ 0 };
        bool readonly{ true };
    };

    template<typename SystemComponentsTuple>
    struct SystemComponentsTupleOperations;

    template<typename... ComponentTypes>
    struct SystemComponentsTupleOperations<std::tuple<ComponentTypes...>>
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
            else if constexpr (traits::arity ==  ComponentArity::Optional)
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
    };

    /// <summary>
    /// Shorthand utility to get all of the SystemComponentInfo for a valid system.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    /// <returns></returns>
    template<ValidSystem System>
    std::vector<SystemComponentInfo> ExtractSystemComponentInfo()
    {
        return SystemComponentsTupleOperations<SystemComponents<System>>::extractComponentInfo();
    }
}

#endif