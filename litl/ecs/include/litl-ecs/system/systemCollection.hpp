#ifndef LITL_ECS_SYSTEM_COLLECTION_H__
#define LITL_ECS_SYSTEM_COLLECTION_H__

#include <memory>
#include <vector>

#include "litl-ecs/system/systemCollectionContext.hpp"
#include "litl-ecs/system/systemGroup.hpp"

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
        SystemCollectionContext addSystem(SystemGroup group) const noexcept
        {
            auto* system = SystemRegistry::getSystem<S>();

            if (!contains(system))
            {
                system->template attach<S>();
                trackSystem(system, group, ExtractSystemComponentInfo<S>());
            }

            return { this, system };
        }

        template<ValidSystem S>
        void contains() const noexcept
        {
            return contains(getSystem<S>());
        }

        bool contains(System const* system) const noexcept;
        void dependsOn(System const* thisSystem, System const* dependsOnThisSystem) const noexcept;
        void placement(System const* system, SystemPlacementHint hint) const noexcept;

    protected:

    private:

        friend class World;

        bool build(World const* world);
        void trackSystem(System* system, SystemGroup group, std::vector<SystemComponentInfo> const& componentInfo) const noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif