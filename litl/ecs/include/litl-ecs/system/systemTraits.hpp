#ifndef LITL_ECS_SYSTEM_TRAITS_H__
#define LITL_ECS_SYSTEM_TRAITS_H__

#include <concepts>
#include <tuple>

#include "litl-core/traits.hpp"
#include "litl-ecs/archetype/chunk.hpp"
#include "litl-ecs/component/component.hpp"

namespace LITL::ECS
{
    class World;

    /// <summary>
    /// The system update methods must begin with "World&,float" and so those are not needed for
    /// custom system parameter decomposition. This removes the always-present mandator parameters.
    /// 
    /// For example: 
    /// 
    ///     update(World& world, float dt, Foo& foo, Bar const& bar)
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
    /// Works with SystemTupleTailImpl to retrieve all but the first two (mandatory World,float) types in the update signature.
    /// </summary>
    /// <typeparam name="Tuple"></typeparam>
    template<typename Tuple>
    using SystemTupleTail = decltype(SystemTupleTailImpl<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple> - 2>{})); 
    //                                                                                    ^ reduce size by 2 so we dont go OOB in the Impl

    /// <summary>
    /// Requirements for a valid System class/struct.
    /// 
    /// All that is needed is there is an "update" method that takes in a World& and float parameter.
    /// Additional parameters can be added and are used for archetype matching and the values are 
    /// provided during system run/iteration.
    /// </summary>
    template<typename S>
    concept ValidSystem = requires
    {
        &S::update;                                                             // must have an "update" method
    }
    && [] {
        using traits = MethodTraits<decltype(&S::update)>;                      // get the traits of the required "update" method
        using args = typename traits::argsTuple;                                // extract the argument types in the update signature

        static_assert(std::tuple_size_v<args> >= 2, "System::update must take atleast: World&, float");

        using Arg0 = std::tuple_element_t<0, args>;                             // first argument type
        using Arg1 = std::tuple_element_t<1, args>;                             // second argument type

        return
            std::same_as<typename traits::returnType, void>&&                   // void return type
            std::same_as<Arg0, World&>&&                                        // first argument: World&
            std::same_as<Arg1, float>;                                          // second argument: float
        }();

    /// <summary>
    /// Retrieves the tuple of types required by the System::update method (excluding the mandatory World&,float).
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<ValidSystem S>
    using SystemComponents = SystemTupleTail<typename MethodTraits<decltype(&S::update)>::argsTuple>;
    //                       ^ remove the first two arguments                            ^ extract the arguments

    /// <summary>
    /// Removes const ref from a type. For example: Foo& -> Foo, Bar const& -> Bar.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    using RemoveConstantValRef = std::remove_cv_t<std::remove_reference_t<T>>;

    template<typename SystemComponentsTuple>
    struct SystemComponentsTupleOperations;

    template<typename... ComponentTypes>
    struct SystemComponentsTupleOperations<std::tuple<ComponentTypes...>>
    {
        static auto extractComponentIds()
        {
            return std::tuple
            {
                ComponentDescriptor::get<RemoveConstantValRef<ComponentTypes>>()->id...
            };
        }

        static auto extractComponentBuffers(Chunk& chunk, ChunkLayout const& layout)
        {
            return std::tuple
            {
                chunk.getRawComponentArray<RemoveConstantValRef<ComponentTypes>>(layout)...
            };
        }
    };
}

#endif