#ifndef LITL_CORE_CONTAINERS_FLAT_HASH_MAP_H__
#define LITL_CORE_CONTAINERS_FLAT_HASH_MAP_H__

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

namespace LITL::Core
{
    /// <summary>
    /// Hash Map that is faster than std::unordered_map (and much faster than std::map).
    /// </summary>
    template<typename K, typename V>
    class FlatHashMap
    {
    public:

        FlatHashMap();
        ~FlatHashMap();

        FlatHashMap(FlatHashMap&&) noexcept;
        FlatHashMap& operator=(FlatHashMap&&) noexcept;

        FlatHashMap(FlatHashMap const&) = delete;
        FlatHashMap& operator=(FlatHashMap const&) = delete;

        /// <summary>
        /// Inserts the new value, if the key is not already in use.
        /// </summary>
        /// <param name="key"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        bool insert(K const& key, V value);

        /// <summary>
        /// Removes the specified key, if it already exists.
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        bool erase(K const& key);

        /// <summary>
        /// Removes all values from the map.
        /// </summary>
        void clear();

        /// <summary>
        /// Returns if the key is present in the map.
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        bool contains(K const& key) const;

        /// <summary>
        /// Returns the value stored at the specified key if it exists.
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        std::optional<std::reference_wrapper<V>> find(K const& key) const;

        /// <summary>
        /// Returns the number of values stored in the map.
        /// </summary>
        /// <returns></returns>
        size_t size() const;
        
    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif