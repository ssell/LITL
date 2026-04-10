#ifndef LITL_CORE_CONTAINERS_PAGED_VECTOR_H__
#define LITL_CORE_CONTAINERS_PAGED_VECTOR_H__

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

namespace litl
{
    /// <summary>
    /// Vector/dynamic array implementation using stable pages.
    /// 
    /// When the container is initialized or the capacity is increased, additional pages/chunks are allocated. Existing pages/elements are
    /// not modified (copied/moved) during allocation in contrast to a standard dynamic array implementation which typically works
    /// on a single large buffer and must move it's elements when a larger buffer is required.
    /// 
    /// Paged vectors are a good alternative for situations such as:
    /// 
    ///   * Large memory requirements. As memory becomes more fragmented during the application life, it can become harder to find a single contiguous
    ///     available block large enough to accommodate certain buffers.
    /// 
    ///   * Stable memory addresses. References to elements within a vector are not stable as they are moved when a standard vector resizes. Pages on
    ///     the otherhand are stable and are unaffected when additional are allocated.
    /// 
    ///   * Sizing up is generally faster for large paged vectors vs standard ones.
    /// 
    /// However standard vectors do still have the edge in a few spaces such as cache locality, internal fragmentation, and random access.
    /// 
    /// Note: as one of the primary uses for this are stable memory addresses, there is (currently) no delete/remove operator available aside from pop_back.
    /// </summary>
    template<typename T, size_t PageSize = 256>
    class PagedVector
    {
    public:

        class PagedVectorIterator
        {
        public:

            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using reference = T&;

            PagedVectorIterator(PagedVector<T, PageSize>* vector, size_t i)
                : m_vector(vector), m_index(i) {}

            reference operator*() const
            {
                return (*m_vector)[m_index];
            }

            PagedVectorIterator& operator++()
            {
                ++m_index;
                return *this;
            }

            PagedVectorIterator& operator--()
            {
                if (m_index > 0) { --m_index; }
                return *this;
            }

            friend bool operator==(PagedVectorIterator const& a, PagedVectorIterator const& b)
            {
                return a.m_index == b.m_index;
            }

            friend bool operator!=(PagedVectorIterator const& a, PagedVectorIterator const& b)
            {
                return a.m_index != b.m_index;
            }

        protected:

        private:

            PagedVector<T, PageSize>* m_vector;
            size_t m_index;
        };

        PagedVector() : m_size(0) {}

        ~PagedVector()
        {
            while (!empty())
            {
                // Ensure everything gets destroyed before the page buffer is deallocated.
                pop_back();
            }
        }

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
                m_pages.emplace_back(std::make_unique<PagedVectorPage>());
            }

            return *std::construct_at(getElementPtr(m_size++), std::forward<Args>(args)...);
        }

        void pop_back()
        {
            if (!empty())
            {
                T* element = getElementPtr(--m_size);
                std::destroy_at(element);
            }
        }

        T& operator[](size_t i)
        {
            assert(m_size > i);
            return *getElementPtr(i);
        }

        T const& operator[](size_t i) const
        {
            assert(m_size > i);
            return *getElementPtr(i);
        }

        T& at(size_t i)
        {
            return *getElementPtr(i);
        }

        T const& at(size_t i) const
        {
            return *getElementPtr(i);
        }

        PagedVectorIterator begin()
        {
            return PagedVectorIterator(this, static_cast<size_t>(0));
        }

        PagedVectorIterator end()
        {
            // One past the end of the container
            return PagedVectorIterator(this, m_size);
        }

        T& front()
        {
            return at(0);
        }

        T const& front() const
        {
            return at(0);
        }

        T& back()
        {
            return at((m_size > 0) ? m_size - 1 : 0);
        }

        T const& back() const
        {
            return at((m_size > 0) ? m_size - 1 : 0);
        }

        void clear()
        {
            m_pages.clear();
            m_size = 0;
        }

    protected:

    private:

        T* getElementPtr(size_t elementIndex) noexcept
        {
            return &m_pages[elementIndex / PageSize].get()->data()[elementIndex % PageSize];
        }

        /// <summary>
        /// A stable pointer to the paged buffer.
        /// </summary>
        struct PagedVectorPage
        {
            PagedVectorPage() 
                : m_pData(std::allocator_traits<std::allocator<T>>::allocate(m_allocator, PageSize))
            {

            }

            ~PagedVectorPage()
            {
                std::allocator_traits<std::allocator<T>>::deallocate(m_allocator, m_pData, PageSize);
            }

            T* data() const noexcept
            {
                return m_pData;
            }
            
        private:

            std::allocator<T> m_allocator;
            T* m_pData;
        };
        
        /// <summary>
        /// When this vector resizes, the internal page pointer remains consistent.
        /// If we stored the page directly in the vector, then when m_pages is resized
        /// our individual page buffer would be recreated. 
        /// </summary>
        std::vector<std::unique_ptr<PagedVectorPage>> m_pages;

        /// <summary>
        /// Total number of elements.
        /// </summary>
        size_t m_size;
    };
}

#endif