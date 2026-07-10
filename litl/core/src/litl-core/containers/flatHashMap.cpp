#include "litl-core/containers/flatHashMap.inl"
#include <string>

// The "default" implementations provided by litl-core.
// More specialized implementations can be provided in other libraries by also including the .inl and defining them.

LITL_INSTANTIATE_FLAT_HASH_MAP(uint32_t, uint32_t);
LITL_INSTANTIATE_FLAT_HASH_MAP(uint32_t, uint64_t);
LITL_INSTANTIATE_FLAT_HASH_MAP(uint32_t, bool);
LITL_INSTANTIATE_FLAT_HASH_MAP(uint64_t, uint64_t);
LITL_INSTANTIATE_FLAT_HASH_MAP(uint64_t, uint32_t);
LITL_INSTANTIATE_FLAT_HASH_MAP(uint64_t, bool);
LITL_INSTANTIATE_FLAT_HASH_MAP(std::string, uint32_t);