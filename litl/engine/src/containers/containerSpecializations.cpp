//#include "litl-core/containers/flatHashMap.inl"
#include "litl-core/containers/flatHashSet.inl"

/**
 * Some of our wrapped containers (especially those that wrap Abseil) provide only an initial 
 * set of "supported" types. This is due to the nature of hiding the third-party dependency while
 * maintaining the use of a template container.
 * 
 * Further specializations are sometimes needed. These are added in this file.
 */

#include "litl-ecs/entity/entity.hpp"
LITL_INSTANTIATE_FLAT_HASH_SET(litl::Entity);