#ifndef LITL_CORE_CONTAINERS_FLAT_HASH_SET_H__
#define LITL_CORE_CONTAINERS_FLAT_HASH_SET_H__

#include <cstdint>
#include <memory>

namespace litl
{
    template<typename T>
    class FlatHashSet
    {
    public:

        FlatHashSet();
        FlatHashSet(FlatHashSet const&) = delete;
        FlatHashSet& operator=(FlatHashSet const&) = delete;

        void insert(T const& v) noexcept;
        [[nodiscard]] bool contains(T const& v) const noexcept;
        bool erase(T const& v) noexcept;
        void clear() noexcept;
        [[nodiscard]] size_t size() const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif