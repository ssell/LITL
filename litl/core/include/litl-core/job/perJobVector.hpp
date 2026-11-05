#ifndef LITL_CORE_JOBS_PER_JOB_VECTOR_H__
#define LITL_CORE_JOBS_PER_JOB_VECTOR_H__

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/constants.hpp"
#include "litl-core/thread.hpp"

namespace litl
{
    /// <summary>
    /// Wrapper around an array of per-thread vectors to be used with the jobs system.
    /// Each job thread gets its own vector which can then be drained/combined into a single vector at a sync point.
    /// </summary>
    template <class T>
    class PerJobVector
    {
        static_assert(!std::is_same_v<T, bool>, "std::vector<bool> is not a real container; use a byte type");
        static constexpr uint32_t JobThreadCount = Constants::max_thread_count + 1;          // +1 for the main thread which can also run jobs while waiting.

    public:

        /// <summary>
        /// Retrieves the vector local to the calling thread. Asserts on the ThreadInfo::index value.
        /// May return nullptr if calling from a non-job non-main thread.
        /// </summary>
        [[nodiscard]] std::vector<T>* local() noexcept
        {
            if (ThreadInfo::get().isMainThread())
            {
                return &m_alignedVectorSlots[Constants::max_thread_count].vector;
            }
            else
            {
                const uint32_t jobIndex = ThreadInfo::get().jobIndex;

                if (jobIndex < Constants::max_thread_count)
                {
                    return &m_alignedVectorSlots[jobIndex].vector;
                }
            }

            return nullptr;
        }

        /// <summary>
        /// Retrieves the vector local to the calling thread. Asserts on the ThreadInfo::index value.
        /// May return nullptr if calling from a non-job non-main thread.
        /// </summary>
        [[nodiscard]] std::vector<T> const* local() const noexcept
        {
            return const_cast<PerJobVector<T>*>(this)->local();
        }

        /// <summary>
        /// Appends the element to the back of the thread-local vector.
        /// Returns true if the item was added, false if it failed. May fail if called from a non-job/non-main thread.
        /// </summary>
        bool push_back(const T& value)
        {
            auto* vector = local();

            if (vector == nullptr)
            {
                return false;
            }

            vector->emplace_back(value);

            return true;
        }

        /// <summary>
        /// Appends the element to the back of the thread-local vector.
        /// Returns true if the item was added, false if it failed. May fail if called from a non-job/non-main thread.
        /// </summary>
        bool push_back(T&& value)
        {
            auto* vector = local();

            if (vector == nullptr)
            {
                return false;
            }

            vector->emplace_back(std::move(value));

            return true;
        }

        /// <summary>
        /// Appends the element to the back of the thread-local vector.
        /// Returns true if the item was added, false if it failed. May fail if called from a non-job/non-main thread.
        /// </summary>
        template <class... Args>
        bool emplace_back(Args&&... args)
        {
            auto* vector = local();

            if (vector == nullptr)
            {
                return false;
            }

            vector->emplace_back(std::forward<Args>(args)...);

            return true;
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
        /// Range over the per-thread vectors themselves.
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
        struct alignas(Constants::cache_line_size) Slot
        {
            std::vector<T> vector;
        };

        std::array<Slot, JobThreadCount> m_alignedVectorSlots{};
    };
}

#endif