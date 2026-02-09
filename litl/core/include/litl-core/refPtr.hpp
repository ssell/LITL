#ifndef LITL_CORE_REF_PTR_H__
#define LITL_CORE_REF_PTR_H__

#include <atomic>
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace LITL::Core
{
    /// <summary>
    /// Default base for classes that want to be used by a RefPtr.
    /// </summary>
    class RefCounted
    {
    public:

        void add() const noexcept
        {
            // Use std::memory_order_relaxed so no ordering/synchronization constraints are enforced.
            // Only the atomicity of this particular operation is guaranteed.
            m_refCount.fetch_add(1, std::memory_order_relaxed);
        }

        void release() const noexcept
        {
            // Use std::memory_order_acq_rel to enforce that no memory read/writes can be done before the store nor after the store. 
            if (m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete this;
            }
        }

    protected:

        RefCounted() = default;
        virtual ~RefCounted() = default;

    private:

        mutable std::atomic<uint32_t> m_refCount{ 1 };
    };

    /// <summary>
    /// Used to enforce that a RefPtr can only wrap a RefCounted.
    /// </summary>
    template<typename T>
    concept IntrusiveRefCounted = std::derived_from<T, RefCounted>;

    /// <summary>
    /// Alternative to std::shared_ptr when we want to have increased lifetime tracking, debugging potential, etc.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template<IntrusiveRefCounted T>
    class RefPtr
    {
    public:

        RefPtr()
            : m_pPtr(nullptr)
        {

        }

        RefPtr(T* ptr) 
            : m_pPtr(ptr) 
        {

        }

        RefPtr(RefPtr const& other) 
            : m_pPtr(other.m_pPtr)
        {
            if (m_pPtr != nullptr)
            {
                m_pPtr->add();
            }
        }

        RefPtr(RefPtr&& other) noexcept
            : m_pPtr(other.m_pPtr)
        {
            other.m_pPtr = nullptr;
        }

        RefPtr& operator=(RefPtr const& other)
        {
            if (this != &other)
            {
                if (m_pPtr != nullptr)
                {
                    m_pPtr->release();
                }

                m_pPtr = other.m_pPtr;

                if (m_pPtr != nullptr)
                {
                    m_pPtr->add();
                }
            }

            return *this;
        }

        RefPtr& operator=(RefPtr&& other) noexcept
        {
            if (m_pPtr != nullptr)
            {
                m_pPtr->release();
            }

            m_pPtr = other.m_pPtr;
            other.m_pPtr = nullptr;

            return *this;
        }

        ~RefPtr()
        {
            if (m_pPtr != nullptr)
            {
                m_pPtr->release();
            }
        }

        T* get() const
        {
            return m_pPtr;
        }

        T* operator->() const
        {
            return m_pPtr;
        }

        T& operator*() const
        {
            return *m_pPtr;
        }

        explicit operator bool() const
        {
            return m_pPtr != nullptr;
        }

    protected:

    private:

        T* m_pPtr = nullptr;
    };

    template<IntrusiveRefCounted T, typename... Args>
    RefPtr<T> make_refptr(Args&&... args)
    {
        return RefPtr<T>(new T(std::forward<Args>(args)...));
    }
}

#endif