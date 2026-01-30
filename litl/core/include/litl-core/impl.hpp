#ifndef LITL_CORE_IMPL_H__
#define LITL_CORE_IMPL_H__

#include <type_traits>
#include <new>

namespace LITL::Core
{
    template<typename T, std::size_t ImplSize, std::size_t ImplAlignment = alignof(std::max_align_t)>
    class ImplPtr
    {
    public:

        ImplPtr()
        {
            static_assert(sizeof(T) <= ImplSize);
            static_assert(alignof(T) <= ImplAlignment);

            new (&m_storage) T();
        }

        ~ImplPtr()
        {
            get()->~T();
        }

        T* get()
        {
            return std::launder(reinterpret_cast<T*>(&m_storage));
        }

        T const* get() const
        {
            return std::launder(reinterpret_cast<T const*>(&m_storage));
        }

        T* operator->()
        {
            return get();
        }

        T const* operator->() const
        {
            return *get();
        }

        T& operator*()
        {
            return *get();
        }

        T const& operator*() const
        {
            return *get();
        }

    protected:

    private:

        alignas(ImplAlignment) std::byte m_storage[ImplSize];
    };
}

#endif