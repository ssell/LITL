#ifndef LITL_ECS_DEFERRED_ENTITY_H__
#define LITL_ECS_DEFERRED_ENTITY_H__

#include <limits>
#include "litl-ecs/constants.hpp"

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
        bool operator==(DeferredEntity const& other)
        {
            return (index == other.index);
        }

        bool operator!=(DeferredEntity const& other)
        {
            return !(*this == other);
        }

        uint32_t index{ Constants::null_entity_id };

        constexpr bool isNull() const noexcept
        {
            return (index == Constants::null_entity_id);
        }
    };
}

#endif