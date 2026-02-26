#ifndef LITL_ENGINE_ECS_SYSTEM_H__
#define LITL_ENGINE_ECS_SYSTEM_H__

#include <memory>
#include <optional>
#include <tuple>
#include <vector>

#include "litl-core/debug.hpp"
#include "litl-ecs/constants.hpp"
#include "litl-ecs/component/component.hpp"
#include "litl-ecs/system/systemRunner.hpp"
#include "litl-ecs/system/systemTraits.hpp"
#include "litl-ecs/system/systemWrapper.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// The result of many levels of wrapping and type erasure.
    /// Stores the user system, system runner, archetype references, etc.
    /// </summary>
    class System : public DebugInfo
    {
    public:

        System();
        ~System();

        SystemTypeId id() const noexcept;

        /// <summary>
        /// Post-instantiation user system type attachment to this System instance.
        /// The user system type is used to compose the SystemRunner, but it is not required to create this System object.
        /// </summary>
        /// <typeparam name="System"></typeparam>
        template<ValidSystem S>
        void attach()
        {
            setDebugName(typeid(S).name());

            m_runFunc = CreateSystemWrapperRunnerTask(SystemWrapper<S>());

            // Get the system components in tuple form. For example: std::tuple<Foo&, Bar&>
            using SystemComponentTuple = SystemComponents<S>;
            registerComponentTypes<SystemComponentTuple>();
        }

        /// <summary>
        /// Alerts the system of new archetypes so that it can bind to any relevant.
        /// </summary>
        /// <param name="newArchetypes"></param>
        void updateArchetypes(std::vector<ArchetypeId> const& newArchetypes) const noexcept;

        /// <summary>
        /// Runs the underyling user system over the provided chunk.
        /// The actual execution is performed by a SystemRunner.
        /// </summary>
        /// <param name="world"></param>
        /// <param name="dt"></param>
        /// <param name="chunk"></param>
        /// <param name="layout"></param>
        void run(World& world, float dt);

    protected:

    private:

        void registerComponentType(ComponentTypeId componentType) const noexcept;

        /// <summary>
        /// Given a tuple of System::update argument types (ie <Foo&, Bar const&>),
        /// extracts out the individual component types (Foo, Bar) and retrieves the
        /// associated component type ids (1, 2) and registers those within the wrapper.
        /// </summary>
        /// <typeparam name="SystemComponentTuple"></typeparam>
        template<typename SystemComponentTuple>
        void registerComponentTypes()
        {
            auto componentTypesTuple = SystemComponentsTupleOperations<SystemComponentTuple>::extractComponentIds();

            std::apply([&](auto&&... componentTypes)
                {
                    (registerComponentType(componentTypes), ...);
                }, componentTypesTuple);
        }


        struct Impl;
        std::unique_ptr<Impl> m_pImpl;

        std::function<LITL::ECS::SystemRunFunc> m_runFunc = nullptr;
    };
}

#endif