#include "litl-ecs/system/systemCollectionContext.hpp"
#include "litl-ecs/system/systemCollection.hpp"
#include "litl-ecs/system/system.hpp"

namespace litl
{
    SystemCollectionContext::SystemCollectionContext(SystemCollection const* collection, System const* system)
        : m_pCollection(collection), m_pSystem(system)
    {
        assert(m_pCollection != nullptr);
        assert(m_pSystem != nullptr);
    }

    SystemCollectionContext const& SystemCollectionContext::dependsOn(System* system) const noexcept
    {
        m_pCollection->dependsOn(m_pSystem, system);
        return (*this);
    }

    SystemCollectionContext const& SystemCollectionContext::placement(SystemPlacementHint hint) const noexcept
    {
        m_pCollection->placement(m_pSystem, hint);
        return (*this);
    }
}