#ifndef LITL_ECS_SYSTEM_TRAITS_H__
#define LITL_ECS_SYSTEM_TRAITS_H__

#include <concepts>
#include <tuple>
#include <vector>

#include "litl-core/traits.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/archetype/chunk.hpp"
#include "litl-ecs/component/component.hpp"

namespace LITL::ECS
{
    class EntityCommands;

    /// <summary>
    /// The system update methods must begin with "EntityCommands&,float" and so those are not needed for
    /// custom system parameter decomposition. This removes the always-present mandator parameters.
    /// 
    /// For example: 
    /// 
    ///     update(EntityCommands& commands, float dt, Foo& foo, Bar const& bar)
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
    //                                                                                    ^ extract [2, 3, 4, ...], skipping [0, 1]

    /// <summary>
    /// Works with SystemTupleTailImpl to retrieve all but the first two (mandatory EntityCommands,float) types in the update signature.
    /// </summary>
    /// <typeparam name="Tuple"></typeparam>
    template<typename Tuple>
    using SystemTupleTail = decltype(SystemTupleTailImpl<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple> - 2>{})); 
    //                                                                                    ^ reduce size by 2 so we dont go OOB in the Impl

    template<typename Tuple, std::size_t... Indices>
    consteval bool ValidSystemComponents(std::index_sequence<Indices...>)
    {
        return (
            (std::is_lvalue_reference_v<std::tuple_element_t<Indices + 2, Tuple>> &&    // all optional ::update arguments must be reference types (either & or const&)
             !std::is_volatile_v<std::tuple_element_t<Indices + 2, Tuple>>) &&          // volatile arguments are not allowed
            ...);
    }

    /// <summary>
    /// Requirements for a valid System class/struct.
    /// 
    /// All that is needed is there is an "update" method that takes in a EntityCommands& and float parameter.
    /// Additional parameters can be added and are used for archetype matching and the values are 
    /// provided during system run/iteration.
    /// </summary>
    template<typename S>
    concept ValidSystem = requires(S s, Core::ServiceProvider& services)
    {
        { s.setup(services) } -> std::same_as<void>;                            // must have a "setup(ServiceProvider& services)" method
        &S::update;                                                             // must have an "update" method (more on that below)
    }
    && [] {
        using traits = MethodTraits<decltype(&S::update)>;                      // get the traits of the required "update" method
        using args = typename traits::argsTuple;                                // extract the argument types in the update signature

        constexpr std::size_t argsCount = std::tuple_size_v<args>;
        constexpr std::size_t componentsCount = std::tuple_size_v<args> -2;

        static_assert(argsCount >= 2, "System::update must take atleast: EntityCommands&, float");
        static_assert((componentsCount == 0) || ValidSystemComponents<args>(std::make_index_sequence<componentsCount>{}), "System::update optional component arguments must be reference or const-reference values only.");

        using Arg0 = std::tuple_element_t<0, args>;                             // first argument type
        using Arg1 = std::tuple_element_t<1, args>;                             // second argument type

        static_assert(std::same_as<typename traits::returnType, void>, "System::update return type must be void.");
        static_assert(std::same_as<Arg0, EntityCommands&>, "System::update first argument must be 'EntityCommands&'");
        static_assert(std::same_as<Arg1, float>, "System::update second argument must be 'float'");

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
        /// Returns a std::tuple of the plain component types expected by the system ::update method.
        /// For example `update(EntityCommands&, float, Foo&, Bar const&)` would return `[Foo, Bar]`
        /// </summary>
        /// <returns></returns>
        static auto extractComponentIds()
        {
            return std::tuple
            {
                ComponentDescriptor::get<std::remove_cvref_t<ComponentTypes>>()->id...
            };
        }

        /// <summary>
        /// Returns a std::tuple of SystemComponentInfo describing the types expected by the system ::update method.
        /// </summary>
        /// <returns></returns>
        static std::vector<SystemComponentInfo> extractComponentInfo()
        {
            std::vector<SystemComponentInfo> componentInfos;

            std::apply([&componentInfos](auto&&... systemComponentInfos)
                {
                    (componentInfos.push_back(systemComponentInfos), ...);
                }, std::tuple
                {
                    SystemComponentInfo{
                        ComponentDescriptor::get<std::remove_cvref_t<ComponentTypes>>()->id,
                        std::is_const_v<std::remove_reference_t<ComponentTypes>>
                        // ^ must remove reference first. the referred-to type is never const, so it "hides" it
                    } ...
                });

            return componentInfos;
        }

        static auto extractComponentBuffers(Chunk& chunk, ChunkLayout const& layout)
        {
            return std::tuple
            {
                chunk.getRawComponentArray<std::remove_cvref_t<ComponentTypes>>(layout)...
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