#ifndef LITL_ECS_SYSTEM_COLLECTION_H__
#define LITL_ECS_SYSTEM_COLLECTION_H__

#include <memory>
#include <vector>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/system/systemGroup.hpp"
#include "litl-ecs/system/systemTraits.hpp"

namespace LITL::ECS
{
    class World;

    /// <summary>
    /// Splits out the "add system" logic from the World so that it is not useable during application run.
    /// </summary>
    class SystemCollection
    {
    public:

        SystemCollection();
        ~SystemCollection();

        SystemCollection(SystemCollection const&) = delete;
        SystemCollection& operator=(SystemCollection const&) = delete;

        /// <summary>
        /// Adds the system, denoted by its type, to to the specified group.
        /// A system can only be added to the manager one time, regardless of group.
        /// </summary>
        /// <typeparam name="S"></typeparam>
        /// <param name="group"></param>
        template<ValidSystem S>
        void addSystem(SystemGroup group) const noexcept
        {
            auto* system = getSystem<S>();

            if (!contains(system))
            {
                system->template attach<S>();
                trackSystem(system, group, ExtractSystemComponentInfo<S>());
            }
        }
        template<ValidSystem S>
        void contains() const noexcept
        {
            return contains(getSystem<S>());
        }


    protected:

    private:

        friend class World;

        bool build(World const* world);

        template<ValidSystem S>
        static System* getSystem()
        {
            static System system;
            return &system;
        }

        bool contains(System* system) const noexcept;
        void trackSystem(System* system, SystemGroup group, std::vector<SystemComponentInfo> const& componentInfo) const noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif