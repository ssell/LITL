#ifndef LITL_ENGINE_ECS_SYSTEM_RUNNER_H__
#define LITL_ENGINE_ECS_SYSTEM_RUNNER_H__

#include <concepts>
#include <tuple>

#include "litl-ecs/constants.hpp"
#include "litl-ecs/archetype/chunk.hpp"

namespace LITL::ECS
{
    class World;
    struct ChunkLayout;

    /// <summary>
    /// Used for member function (update function specifically) parameter decomposition.
    /// </summary>
    /// <typeparam name=""></typeparam>
    template<typename>
    struct SystemMemberFunctionTraits;

    /// <summary>
    /// Non-const overload.
    /// </summary>
    /// <typeparam name="C"></typeparam>
    /// <typeparam name="R"></typeparam>
    /// <typeparam name="...Args"></typeparam>
    template<typename C, typename R, typename... Args>
    struct SystemMemberFunctionTraits<R(C::*)(Args...)>
    {
        using class_type = C;
        using return_type = R;
        using args_tuple = std::tuple<Args...>;
    };

    /// <summary>
    /// Const overload.
    /// </summary>
    /// <typeparam name="C"></typeparam>
    /// <typeparam name="R"></typeparam>
    /// <typeparam name="...Args"></typeparam>
    template<typename C, typename R, typename... Args>
    struct SystemMemberFunctionTraits<R(C::*)(Args...) const>
    {
        using class_type = C;
        using return_type = R;
        using args_tuple = std::tuple<Args...>;
    };

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

    /// <summary>
    /// Works with SystemTupleTailImpl.
    /// </summary>
    /// <typeparam name="Tuple"></typeparam>
    template<typename Tuple>
    using SystemTupleTail = decltype(SystemTupleTailImpl<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple> - 2>{}));

    /// <summary>
    /// Requirements for a valid System class/struct.
    /// 
    /// All that is needed is there is an "update" method that takes in a World& and float parameter.
    /// Additional parameters can be added and are used for archetype matching and the values are 
    /// provided during system run/iteration.
    /// </summary>
    template<typename T>
    concept ValidSystem =
        requires
    {
        &T::update;
    }
    && [] {
        using traits = SystemMemberFunctionTraits<decltype(&T::update)>;
        using args = typename traits::args_tuple;

        static_assert(std::tuple_size_v<args> >= 2, "System::update must take atleast: World&, float");

        using A0 = std::tuple_element_t<0, args>;
        using A1 = std::tuple_element_t<1, args>;

        return
            std::same_as<typename traits::return_type, void>&&
            std::same_as<A0, World&>&&
            std::same_as<A1, float>;
        }();

    /// <summary>
    /// Retrieves the tuple of types required by the system (excluding the mandatory World&,float).
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<ValidSystem T>
    using SystemComponents = SystemTupleTail<typename SystemMemberFunctionTraits<decltype(&T::update)>::args_tuple>;

    /// <summary>
    /// Removes const ref from a type. For example: Foo& -> Foo, Bar const& -> Bar.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<typename T>
    using RemoveConstantValRef = std::remove_cv_t<std::remove_reference_t<T>>;

    /// <summary>
    /// The function signature required to run the SystemRunner.
    /// </summary>
    using SystemRunFunc = void(LITL::ECS::World&, float, LITL::ECS::Chunk&, LITL::ECS::ChunkLayout const&);

    /// <summary>
    /// Responsible for running a system over a single archetype chunk.
    /// The chunk is expected to have all of the components required by the system.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    template<ValidSystem System>
    class SystemRunner
    {
    public:

        SystemRunner(System* system)
            : m_pSystem(system)
        {

        }

        // Must match SystemRunFunc
        void run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            // Get the system components in tuple form. For example: std::tuple<Foo&, Bar&>
            using SystemComponentTuple = SystemComponents<System>;
            iterate<SystemComponentTuple>(world, dt, chunk, layout);
        }

    protected:

    private:

        template<typename SystemComponentTuple>
        void iterate(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            // Retrieve the data ptr for each component in the tuple type.
            // For example: SystemComponentTuple -> std::tuple<Foo&, Bar&> ->
            //    componentArrays[0] = Foo*
            //    componentArrays[1] = Bar*
            auto componentArrays = [&]<typename... ComponentTypes>(std::tuple<ComponentTypes...>*)
            {
                // Note we remove the reference when getting the array. Example: "Foo const&" -> "Foo"
                return std::tuple{ chunk.getRawComponentArray<RemoveConstantValRef<ComponentTypes>>(layout)... };
            }((SystemComponentTuple*)nullptr);

            const uint32_t entityCount = chunk.size();

            // Call System::update for each entity in the chunk.
            for (uint32_t i = 0; i < entityCount; ++i)
            {
                // Use apply to expand the tuple into parameters.
                // Example: (Foo, Bar) -> lambda(Foo*, Bar*)
                std::apply([&](auto&... componentArray)
                    {
                        m_pSystem->update(world, dt, componentArray[i]...);
                    }, componentArrays);
            }
        }

        System* m_pSystem;
    };
}

#endif