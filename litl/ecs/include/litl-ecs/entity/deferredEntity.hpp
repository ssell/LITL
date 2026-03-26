#ifndef LITL_ECS_DEFERRED_ENTITY_H__
#define LITL_ECS_DEFERRED_ENTITY_H__

#include <limits>

namespace LITL::ECS
{
    /// <summary>
    /// Represents an entity that does not yet exists.
    /// 
    /// It is used when an entity is requested to be created by through the deferred command buffer
    /// so that components may be added (or removed) from it before it ever exists. At the next 
    /// command sync point the deferred entity is transformed into a standard entity.
    /// 
    /// Deferred entities have no version, and their indices are local to the command buffer
    /// that creates and operates on it. Deferred entities are not visible to any logic other
    /// than the immediate context in which they are requested, and can not be operated upon
    /// by any other system (or instance of the same system) until they are materialized.
    /// </summary>
    struct DeferredEntity
    {
        constexpr static uint32_t NullIndex = std::numeric_limits<uint32_t>::max();
        uint32_t index{ NullIndex };

        constexpr bool isNull() const noexcept
        {
            return (index == NullIndex);
        }
    };
}

#endif