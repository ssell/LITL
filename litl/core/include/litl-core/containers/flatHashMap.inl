#include <absl/container/flat_hash_map.h>
#include "litl-core/containers/flatHashMap.hpp"

// ... todo ... hmmmmmm ...
// not sure if flat_hash_map is wanted here, but actually node_hash_map.
// since flat_hash_map invalidates iterators and references on rehash (manually or triggered by growth).
// this is being used by various registries/caches, but the question is if the underlying addresses are being referenced.
// need to do a review and see ... 
// if registries/caches start breaking, then look here!

namespace litl
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
    bool FlatHashMap<K, V>::insert(K const& key, V value) noexcept
    {
        // map.emplace returns <iterator, bool> where the bool is the insertion success
        return m_pImpl->map.emplace(key, std::move(value)).second;
    }

    template<typename K, typename V>
    bool FlatHashMap<K, V>::erase(K const& key) noexcept
    {
        return m_pImpl->map.erase(key);
    }

    template<typename K, typename V>
    void FlatHashMap<K, V>::clear() noexcept
    {
        m_pImpl->map.clear();
    }

    template<typename K, typename V>
    bool FlatHashMap<K, V>::contains(K const& key) const noexcept
    {
        return m_pImpl->map.contains(key);
    }

    template<typename K, typename V>
    std::optional<std::reference_wrapper<V>> FlatHashMap<K, V>::find(K const& key) noexcept
    {
        auto iter = m_pImpl->map.find(key);

        if (iter == m_pImpl->map.end())
        {
            return std::nullopt;
        }

        return iter->second;
    }

    template<typename K, typename V>
    size_t FlatHashMap<K, V>::size() const noexcept
    {
        return m_pImpl->map.size();
    }
}

#define LITL_INSTANTIATE_FLAT_HASH_MAP(K, V) template class litl::FlatHashMap<K, V>;