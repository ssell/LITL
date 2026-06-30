#include "litl-core/containers/flatHashSet.inl"

// The "default" implementations provided by litl-core.
// More specialized implementations can be provided in other libraries by also including the .inl and defining them.

LITL_INSTANTIATE_FLAT_HASH_SET(uint32_t);
LITL_INSTANTIATE_FLAT_HASH_SET(uint64_t);