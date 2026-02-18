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
    std::optional<V> FlatHashMap<K, V>::find(K const& key) const
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
    // Implementations
    // -------------------------------------------------------------------------------------

    template class FlatHashMap<uint32_t, uint32_t>;
}