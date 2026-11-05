#ifndef LITL_CORE_CONTAINERS_PER_THREAD_VECTOR_H__
#define LITL_CORE_CONTAINERS_PER_THREAD_VECTOR_H__

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/constants.hpp"
#include "litl-core/thread.hpp"

namespace litl
{
    template <class T, size_t MaxThreads = Constants::max_thread_count>
    class PerThreadVector
    {
        static_assert(MaxThreads > 0ull);
        static_assert(!std::is_same_v<T, bool>, "std::vector<bool> is not a real container; use a byte type");

    public:

        /// <summary>
        /// Retrieves the vector local to the calling thread. Asserts on the ThreadInfo::index value.
        /// </summary>
        [[nodiscard]] std::vector<T>& local(this auto& self) noexcept
        {
            LITL_FATAL_ASSERT(ThreadInfo::get().index < MaxThreads);
            return self.m_alignedVectorSlots[ThreadInfo::get().index].vector;
        }

        /// <summary>
        /// Appends the element to the back of the thread-local vector.
        /// </summary>
        T& push_back(const T& v)
        {
            return local().emplace_back(v);
        }

        /// <summary>
        /// Appends the element to the back of the thread-local vector.
        /// </summary>
        T& push_back(T&& v)
        {
            return local().emplace_back(std::move(v));
        }

        /// <summary>
        /// Appends the element to the back of the thread-local vector.
        /// </summary>
        template <class... Args>
        T& emplace_back(Args&&... args)
        {
            return local().emplace_back(std::forward<Args>(args)...);
        }

        /// <summary>
        /// Reserves capacity into each thread-local vector.
        /// This should be called only from the main thread/sync point.
        /// </summary>
        void reserve_each(size_t per_thread_capacity)
        {
            for (auto& slot : m_alignedVectorSlots)
            {
                slot.vector.reserve(per_thread_capacity);
            }
        }

        /// <summary>
        /// Returns the total number of elements across all per-thread vectors.
        /// This should be called only from the main thread/sync point.
        /// </summary>
        [[nodiscard]] size_t size() const noexcept
        {
            size_t total = 0ull;

            for (auto& slot : m_alignedVectorSlots)
            {
                total += slot.vector.size();
            }

            return total;
        }

        /// <summary>
        /// Returns true if all per-thread vectors are empty.
        /// This should be called only from the main thread/sync point.
        /// </summary>
        [[nodiscard]] bool empty() const noexcept
        {
            return std::ranges::all_of(m_alignedVectorSlots, [](const Slot& slot) { return slot.vector.empty(); });
        }

        /// <summary>
        /// Invokes clear on all per-thread vectors.
        /// This should be called only from the main thread/sync point.
        /// </summary>
        void clear() noexcept
        {
            for (auto& slot : m_alignedVectorSlots)
            {
                slot.vector.clear();
            }
        }

        /// <summary>
        /// This should be called only from the main thread/sync point.
        /// </summary>
        void shrink_to_fit()
        {
            for (auto& slot : m_alignedVectorSlots)
            {
                slot.vector.shrink_to_fit();
            }
        }

        /// <summary>
        /// Range over the per-thread vectors themselves (size MaxThreads).
        /// This should be called only from the main thread/sync point.
        /// </summary>
        [[nodiscard]] auto vectors(this auto& self) noexcept
        {
            return self.m_alignedVectorSlots | std::views::transform([](auto& slot) -> auto& { return slot.vector; });
        }

        /// <summary>
        /// Zero-copy flattened view, ordered by thread index then insertion order.
        /// Single-pass-ish iteration; no random access. Invalidated by any write.
        /// This should be called only from the main thread/sync point.
        /// </summary>
        [[nodiscard]] auto view(this auto& self) noexcept
        {
            return self.vectors() | std::views::join;
        }

        /// <summary>
        /// This should be called only from the main thread/sync point.
        /// </summary>
        [[nodiscard]] std::vector<T> collect() const
        {
            std::vector<T> out;
            out.reserve(size());

            for (const auto& slot : m_alignedVectorSlots)
            {
                out.insert(out.end(), slot.vector.begin(), slot.vector.end());
            }

            return out;
        }

        /// <summary>
        // Moves all values into the specified vector and then clear the slots (capacity retained).
        /// This should be called only from the main thread/sync point.
        /// </summary>
        void drain_into(std::vector<T>& dst)
        {
            const size_t needed = dst.size() + size();

            if (dst.capacity() < needed)
            {
                dst.reserve(std::max(needed, dst.capacity() * 2));          // geometric guard for repeated appends
            }

            for (auto& slot : m_alignedVectorSlots)
            {
                dst.insert(dst.end(), std::make_move_iterator(slot.vector.begin()), std::make_move_iterator(slot.vector.end()));
            }

            clear();
        }

        /// <summary>
        /// Drains all values into a new vector and returns it.
        /// This should be called only from the main thread/sync point.
        /// </summary>
        [[nodiscard]] std::vector<T> drain()
        {
            std::vector<T> out;
            drain_into(out);
            return out;
        }

    private:

        // Pad each slot to a cache line so threads pushing to adjacent slots don't ping-pong the vector headers (begin/end/cap pointers get written on every push_back).
        struct alignas(64) Slot
        {
            std::vector<T> vector;
        };

        std::array<Slot, MaxThreads> m_alignedVectorSlots{};
    };
}

#endif