#include <absl/container/flat_hash_map.h>
#include "litl-core/containers/flatHashMap.hpp"

namespace LITL::Core
{
    template<typename K, typename V>
    struct FlatHashMap<K, V>::Impl
    {
        absl::flat_hash_map<K, V> map;
    };

    template<typename K, typename V>
    FlatHashMap<K, V>::FlatHashMap()
        : m_pImpl(std::make_unique<Impl>())
    {
        
    }

    template<typename K, typename V>
    FlatHashMap<K, V>::~FlatHashMap()
    {
        
    }

    template<typename K, typename V>
    bool FlatHashMap<K, V>::insert(K const& key, V value)
    {
        // map.emplace returns <iterator, bool> where the bool is the insertion success
        return m_pImpl->map.emplace(key, std::move(value)).second;
    }

    template<typename K, typename V>
    bool FlatHashMap<K, V>::erase(K const& key)
    {
        return m_pImpl->map.erase(key);
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::clear()
    {
        m_pImpl->map.clear();
    }

    template<typename K, typename V>
    bool FlatHashMap<K, V>::contains(K const& key) const
    {
        return m_pImpl->map.contains(key);
    }

    template<typename K, typename V>
    std::optional<std::reference_wrapper<V>> FlatHashMap<K, V>::find(K const& key) const
    {
        auto iter = m_pImpl->map.find(key);

        if (iter == m_pImpl->map.end())
        {
            return std::nullopt;
        }

        return iter->second;
    }

    template<typename K, typename V>
    size_t FlatHashMap<K, V>::size() const
    {
        return m_pImpl->map.size();
    }

    // -------------------------------------------------------------------------------------
    // To keep the implementation private (and not expose absl::flat_hash_map) the trade off
    // is that FlashHashMap only works on the pre-declared specializations below. This is
    // fine for now as it is only intended for internal usage.
    // -------------------------------------------------------------------------------------

    template class FlatHashMap<uint32_t, uint32_t>;
    template class FlatHashMap<uint32_t, uint64_t>;
    template class FlatHashMap<uint64_t, uint64_t>;
    template class FlatHashMap<uint64_t, uint32_t>;
}