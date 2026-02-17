#ifndef LITL_CORE_CONTAINERS_PAGED_VECTOR_H__
#define LITL_CORE_CONTAINERS_PAGED_VECTOR_H__

#include <cstdint>
#include <memory>
#include <stdexcept>

namespace LITL::Core
{
    /// <summary>
    /// Vector/dynamic array implementation using stable pages.
    /// 
    /// When the container is initialized or the capacity is increased,
    /// additional pages/chunks are allocated. Existing pages/elements are
    /// not modified (copied/moved) during allocation in contrast to a standard
    /// dynamic array implementation which typically works on a single large
    /// buffer and must move it's elements when a larger buffer is required.
    /// 
    /// Paged vectors are a good alternative for situations such as:
    /// 
    ///   * Large memory requirements. As memory becomes more fragmented during 
    /// the application life, it can become harder to find a single contiguous
    /// available block large enough to accomate certain buffers.
    /// 
    ///   * Stable memory addresses. References to elements within a vector are
    /// not stable as they are moved when a standard vector resizes. Pages on
    /// the otherhand are stable and are unaffected when additional are allocated.
    /// 
    ///   * Sizing up is generally faster for large paged vectors vs standard ones.
    /// 
    /// However standard vectors do still have the edge in a few spaces such as
    /// cache locality, internal fragmentation, and random access.
    /// </summary>
    template<typename T, size_t PageSize = 256>
    class PagedVector
    {
        static_assert(PageSize > 0);

    public:

        PagedVector() : m_size(0) {}

        size_t size() const noexcept
        {
            return m_size;
        }

        size_t capacity() const noexcept
        {
            return m_pages.size() * PageSize;
        }

        bool empty() const noexcept
        {
            return m_size == 0;
        }

        bool full() const noexcept
        {
            return m_size == (m_pages.size() * PageSize);
        }

        void push_back(T const& value)
        {
            emplace_back(value);
        }

        void push_back(T&& value)
        {
            emplace_back(std::move(value));
        }

        template<class... Args>
        T& emplace_back(Args&&... args)
        {
            if (full())
            {
                m_pages.emplace_back();
            }

            const size_t index = m_size++;
            T* element = getElementPtr(index);
            std::construct_at(element, std::forward<Args>(args)...);

            return *element;
        }

        void pop_back()
        {
            if (!empty())
            {
                const size_t index = --m_size;
                T* element = getElementPtr(index);
                std::destroy_at(element);
            }
        }

        T& operator[](size_t i) noexcept
        {
            if (i >= m_size) throw std::out_of_range("PagedVector");
            return *getElementPtr(i);
        }

        T const& operator[](size_t i) const noexcept
        {
            if (i >= m_size) throw std::out_of_range("PagedVector");
            return *getElementPtr(i);
        }

        T& at(size_t i)
        {
            if (i >= m_size) throw std::out_of_range("PagedVector");
            return *getElementPtr(i);
        }

    protected:

    private:

        /// <summary>
        /// Returns the index of the page in which the element resides.
        /// </summary>
        /// <param name="elementIndex"></param>
        /// <returns></returns>
        static constexpr size_t getPageIndex(size_t elementIndex) noexcept
        {
            return elementIndex / PageSize;
        }

        /// <summary>
        /// Returns the index inside of the page for where the element resides.
        /// </summary>
        /// <param name="elementIndex"></param>
        /// <returns></returns>
        static constexpr size_t getPageOffset(size_t elementIndex) noexcept
        {
            return elementIndex % PageSize;
        }

        T* getElementPtr(size_t elementIndex) noexcept
        {
            return &m_pages[getPageIndex(elementIndex)].dataPtr[getPageOffset(elementIndex)];
        }

        /// <summary>
        /// A stable pointer to the paged buffer.
        /// </summary>
        struct PagedVectorPage
        {
            PagedVectorPage() : dataPtr(std::make_unique<T[]>(PageSize)) {}
            std::unique_ptr<T[]> dataPtr;
        };
        
        /// <summary>
        /// When this vector resizes, the internal page pointer remains consistent.
        /// </summary>
        std::vector<PagedVectorPage> m_pages;

        /// <summary>
        /// Total number of elements.
        /// </summary>
        size_t m_size;
    };
}

#endif