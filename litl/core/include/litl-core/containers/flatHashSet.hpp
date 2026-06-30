#ifndef LITL_CORE_CONTAINERS_FLAT_HASH_SET_H__
#define LITL_CORE_CONTAINERS_FLAT_HASH_SET_H__

#include <cstdint>
#include <memory>
#include <absl/container/flat_hash_set.h>

namespace litl
{
    template<typename T>
    class FlatHashSet
    {
    public:

        using iterator = typename absl::flat_hash_set<T>::iterator;
        using const_iterator = typename absl::flat_hash_set<T>::const_iterator;

        FlatHashSet() = default;
        FlatHashSet(FlatHashSet const&) = delete;
        FlatHashSet& operator=(FlatHashSet const&) = delete;

        void insert(T const& v) noexcept { m_set.insert(v); }
        [[nodiscard]] bool contains(T const& v) const noexcept { return m_set.contains(v); }
        bool erase(T const& v) noexcept { return m_set.erase(v); }
        void clear() noexcept { m_set.clear(); }

        [[nodiscard]] size_t size() const noexcept { return m_set.size(); }
        [[nodiscard]] bool empty() const noexcept { return m_set.empty(); }

        iterator begin() noexcept { return m_set.begin(); }
        iterator end() noexcept { return m_set.end(); }
        const_iterator begin() const noexcept { return m_set.begin(); }
        const_iterator end() const noexcept { return m_set.end(); }

    private:

        absl::flat_hash_set<T> m_set;
    };
}

#endif