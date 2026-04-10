#ifndef LITL_CORE_CONTAINERS_PAGED_VECTOR_H__
#define LITL_CORE_CONTAINERS_PAGED_VECTOR_H__

#include <cassert>
#include <cstdint>
#include <memory>
#include <type_traits>
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
        static_assert(PageSize > 0, "PageSize must be greater than zero");

    public:

        // ---------------------------------------------------------------------------------
        // Iterator
        // ---------------------------------------------------------------------------------

        template<bool IsConst>
        class PagedVectorIterator
        {
        public:

            using iterator_category = std::random_access_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = std::conditional_t<IsConst, const T*, T*>;
            using reference = std::conditional_t<IsConst, const T&, T&>;

        private:

            using vector_ptr = std::conditional_t<IsConst, const PagedVector*, PagedVector*>;
            vector_ptr m_vector;
            size_t     m_index;

        public:

            PagedVectorIterator() : m_vector(nullptr), m_index(0) {}
            PagedVectorIterator(vector_ptr vector, size_t i) : m_vector(vector), m_index(i) {}
            PagedVectorIterator(PagedVectorIterator<false> const& other) : m_vector(other.m_vector), m_index(other.m_index) {}
            PagedVectorIterator(PagedVectorIterator<true> const& other) requires IsConst : m_vector(other.m_vector), m_index(other.m_index) {}

            reference operator*()  const { return (*m_vector)[m_index]; }
            pointer   operator->() const { return &(*m_vector)[m_index]; }

            PagedVectorIterator& operator++() { ++m_index; return *this; }
            PagedVectorIterator  operator++(int) { auto tmp = *this; ++m_index; return tmp; }
            PagedVectorIterator& operator--() { --m_index; return *this; }
            PagedVectorIterator  operator--(int) { auto tmp = *this; --m_index; return tmp; }

            PagedVectorIterator& operator+=(difference_type n) { m_index += n; return *this; }
            PagedVectorIterator& operator-=(difference_type n) { m_index -= n; return *this; }

            PagedVectorIterator operator+(difference_type n) const { return { m_vector, m_index + n }; }
            PagedVectorIterator operator-(difference_type n) const { return { m_vector, m_index - n }; }

            friend PagedVectorIterator operator+(difference_type n, PagedVectorIterator const& it) { return it + n; }

            difference_type operator-(PagedVectorIterator const& other) const
            {
                return static_cast<difference_type>(m_index) - static_cast<difference_type>(other.m_index);
            }

            reference operator[](difference_type n) const { return *(*this + n); }

            auto operator<=>(PagedVectorIterator const& other) const = default;

            friend class PagedVectorIterator<!IsConst>;
        };

        using iterator = PagedVectorIterator<false>;
        using const_iterator = PagedVectorIterator<true>;

        // ---------------------------------------------------------------------------------
        // PagedVector Implementation
        // ---------------------------------------------------------------------------------

        PagedVector() : m_size(0) {}

        ~PagedVector()
        {
            clear();
        }

        PagedVector(PagedVector&& other)
            : m_pages(std::move(other.m_pages)), m_size(other.m_size)
        {
            other.m_size = 0;
        }

        PagedVector& operator=(PagedVector&& other)
        {
            if (this != &other)
            {
                clear();
                m_pages = std::move(other.m_pages);
                m_size = other.m_size;
                other.m_size = 0;
            }

            return *this;
        }

        // Not copyable, but can be moved
        PagedVector(PagedVector const&) = delete;
        PagedVector& operator=(PagedVector const&) = delete; 

        [[nodiscard]] size_t size() const noexcept
        {
            return m_size;
        }

        [[nodiscard]] size_t capacity() const noexcept
        {
            return m_pages.size() * PageSize;
        }

        [[nodiscard]] bool empty() const noexcept
        {
            return m_size == 0;
        }

        [[nodiscard]] bool full() const noexcept
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

            // Make sure to create and then increment m_size
            T* ptr = getElementPtr(m_size);
            std::construct_at(ptr, std::forward<Args>(args)...);
            m_size++;

            return *ptr;
        }

        void pop_back()
        {
            if (!empty())
            {
                T* element = getElementPtr(--m_size);
                std::destroy_at(element);
            }
        }

        [[nodiscard]] T& operator[](size_t i)
        {
            assert(m_size > i);
            return *getElementPtr(i);
        }

        [[nodiscard]] T const& operator[](size_t i) const
        {
            assert(m_size > i);
            return *getElementPtr(i);
        }

        [[nodiscard]] T& at(size_t i)
        {
            return (*this)[i];
        }

        [[nodiscard]] T const& at(size_t i) const
        {
            return (*this)[i];
        }

        void reserve(size_t count)
        {
            // Keep adding pages until we can store AT LEAST the specified the number of elements.
            while (capacity() < count)
            {
                m_pages.emplace_back(std::make_unique<PagedVectorPage>());
            }
        }

        void clear()
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                // if not trivial, then we have to pop each so the destructor gets called
                while (!empty()) 
                { 
                    pop_back(); 
                }
            }

            m_pages.clear();
            m_size = 0;
        }

        [[nodiscard]] iterator begin()              { return { this, 0 }; }
        [[nodiscard]] iterator end()                { return { this, m_size }; }
        [[nodiscard]] const_iterator begin() const  { return { this, 0 }; }
        [[nodiscard]] const_iterator end() const    { return { this, m_size }; }
        [[nodiscard]] const_iterator cbegin() const { return begin(); }
        [[nodiscard]] const_iterator cend() const   { return end(); }
        [[nodiscard]] T& front()                    { assert(!empty()); return at(0); }
        [[nodiscard]] T const& front() const        { assert(!empty()); return at(0); }
        [[nodiscard]] T& back()                     { assert(!empty()); return at(m_size - 1); }
        [[nodiscard]] T const& back() const         { assert(!empty()); return at(m_size - 1); }

    protected:

    private:

        [[nodiscard]] T* getElementPtr(size_t elementIndex) noexcept
        {
            return &m_pages[elementIndex / PageSize]->data()[elementIndex % PageSize];
        }

        [[nodiscard]] T const* getElementPtr(size_t elementIndex) const noexcept
        {
            return &m_pages[elementIndex / PageSize]->data()[elementIndex % PageSize];
        }

        // ---------------------------------------------------------------------------------
        // PagedVectorPage
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// A stable pointer to the paged buffer.
        /// </summary>
        struct PagedVectorPage
        {
            PagedVectorPage() : m_pData(std::allocator<T>{}.allocate(PageSize)) { }
            ~PagedVectorPage() { std::allocator<T>{}.deallocate(m_pData, PageSize); }
            PagedVectorPage(PagedVectorPage const&) = delete;
            PagedVectorPage& operator=(PagedVectorPage const&) = delete;

            [[nodiscard]] T* data() const noexcept { return m_pData; }
            
        private:

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

        static_assert(std::random_access_iterator<iterator>);
        static_assert(std::random_access_iterator<const_iterator>);
    };
}

#endif