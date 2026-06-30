#include <absl/container/flat_hash_set.h>
#include "litl-core/containers/flatHashSet.hpp"

namespace litl
{
    template<typename T>
    struct FlatHashSet<T>::Impl
    {
        absl::flat_hash_set<T> set;
    };

    template<typename T>
    FlatHashSet<T>::FlatHashSet()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    template<typename T>
    void FlatHashSet<T>::insert(T const& v) noexcept
    {
        m_pImpl->set.insert(v);
    }

    template<typename T>
    bool FlatHashSet<T>::contains(T const& v) const noexcept
    {
        return m_pImpl->set.contains(v);
    }

    template<typename T>
    bool FlatHashSet<T>::erase(T const& v) noexcept
    {
        return false;
    }

    template<typename T>
    void FlatHashSet<T>::clear() noexcept
    {

    }

    template<typename T>
    size_t FlatHashSet<T>::size() const noexcept
    {
        return 0ull;
    }
}

#define LITL_INSTANTIATE_FLAT_HASH_SET(T) template class litl::FlatHashSet<T>;