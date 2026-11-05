#include <algorithm>
#include <atomic>
#include <cstdint>
#include <memory>
#include <ranges>
#include <thread>
#include <utility>
#include <vector>

#include "tests.hpp"
#include "litl-core/thread.hpp"
#include "litl-core/job/jobScheduler.hpp"
#include "litl-core/job/perJobVector.hpp"

namespace litl::tests
{
    namespace
    {
        /// PerJobVector routes main-thread pushes to its dedicated slot, and JobScheduler::wait()
        /// executes jobs on the calling thread, so the test thread must be the claimed main thread.
        /// setMainThread is first-caller-wins; fail loudly if something else already claimed it.
        void claimMainThread()
        {
            ThreadInfo::setMainThread();
            REQUIRE(ThreadInfo::isMainThread());
        }

        struct Entry
        {
            uint32_t job;
            uint32_t seq;
        };
    }

    // -------------------------------------------------------------------------------------
    // Construction / layout
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("Default Constructed Is Empty", "[core::job::perJobVector]")
    {
        PerJobVector<int> v;

        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);
        REQUIRE(v.collect().empty());
        REQUIRE(std::ranges::distance(v.view()) == 0);
        REQUIRE(std::ranges::size(v.vectors()) == static_cast<size_t>(Constants::max_thread_count) + 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Slots Are Cache Line Aligned", "[core::job::perJobVector]")
    {
        PerJobVector<int> v;

        for (auto const& vec : v.vectors())
        {
            REQUIRE(reinterpret_cast<std::uintptr_t>(&vec) % Constants::cache_line_size == 0);
        }
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Single-thread (main) behavior
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("Main Thread Push And Emplace", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<std::pair<int, int>> v;
        REQUIRE(v.local() != nullptr);

        const std::pair<int, int> lvalue{ 1, 1 };
        REQUIRE(v.push_back(lvalue));                       // const T&
        REQUIRE(v.push_back(std::pair<int, int>{ 2, 2 }));  // T&&
        REQUIRE(v.emplace_back(3, 3));                      // forwarded ctor args

        REQUIRE(v.size() == 3);
        REQUIRE_FALSE(v.empty());

        const std::vector<std::pair<int, int>> expected{ { 1, 1 }, { 2, 2 }, { 3, 3 } };
        REQUIRE(*v.local() == expected);
        REQUIRE(v.collect() == expected);

        // Everything landed in exactly one slot: the caller's.
        const auto nonEmpty = std::ranges::count_if(v.vectors(), [](auto const& vec) { return !vec.empty(); });
        REQUIRE(nonEmpty == 1);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Local Is Stable For The Same Thread", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        auto* first = v.local();
        v.push_back(1);

        REQUIRE(v.local() == first);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Collect Does Not Modify", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        v.push_back(1);
        v.push_back(2);

        std::ignore = v.collect();

        REQUIRE(v.size() == 2);
        REQUIRE(*v.local() == std::vector<int>{ 1, 2 });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("View Matches Collect And Is Writable", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        for (int i = 0; i < 5; ++i)
        {
            v.push_back(i);
        }

        std::vector<int> viewed;
        for (int x : v.view())
        {
            viewed.push_back(x);
        }

        REQUIRE(viewed == v.collect());

        for (int& x : v.view())
        {
            x *= 10;
        }

        REQUIRE(v.collect() == std::vector<int>{ 0, 10, 20, 30, 40 });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Clear Empties And Retains Capacity", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        for (int i = 0; i < 10; ++i)
        {
            v.push_back(i);
        }

        const auto capacity = v.local()->capacity();
        v.clear();

        REQUIRE(v.empty());
        REQUIRE(v.size() == 0);
        REQUIRE(v.local()->capacity() == capacity);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Reserve Each Reserves Every Slot", "[core::job::perJobVector]")
    {
        PerJobVector<int> v;
        v.reserve_each(64);

        for (auto const& vec : v.vectors())
        {
            REQUIRE(vec.capacity() >= 64);
        }

        REQUIRE(v.empty());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Shrink To Fit Preserves Contents", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        v.reserve_each(128);
        v.push_back(7);
        v.push_back(8);

        v.shrink_to_fit();      // non-binding request; only contents are guaranteed

        REQUIRE(v.collect() == std::vector<int>{ 7, 8 });
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Drain
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("Drain Into Appends And Clears", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        const auto capacity = v.local()->capacity();

        std::vector<int> dst{ -1, -2 };
        v.drain_into(dst);

        REQUIRE(dst == std::vector<int>{ -1, -2, 1, 2, 3 });
        REQUIRE(v.empty());
        REQUIRE(v.local()->capacity() == capacity);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Drain Returns Contents And Container Is Reusable", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        REQUIRE(v.drain().empty());     // draining empty is fine

        v.push_back(1);
        v.push_back(2);
        REQUIRE(v.drain() == std::vector<int>{ 1, 2 });
        REQUIRE(v.empty());

        v.push_back(3);
        REQUIRE(v.drain() == std::vector<int>{ 3 });
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Move Only Types", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<std::unique_ptr<int>> v;
        REQUIRE(v.push_back(std::make_unique<int>(7)));
        REQUIRE(v.emplace_back(std::make_unique<int>(8)));

        auto out = v.drain();

        REQUIRE(out.size() == 2);
        REQUIRE(*out[0] == 7);
        REQUIRE(*out[1] == 8);
        REQUIRE(v.empty());
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Const access
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("Const Access", "[core::job::perJobVector]")
    {
        claimMainThread();

        PerJobVector<int> v;
        v.push_back(4);
        v.push_back(5);

        PerJobVector<int> const& cv = v;

        REQUIRE(cv.size() == 2);
        REQUIRE_FALSE(cv.empty());
        REQUIRE(cv.collect() == std::vector<int>{ 4, 5 });
        REQUIRE(std::ranges::distance(cv.view()) == 2);
        REQUIRE(std::ranges::size(cv.vectors()) == static_cast<size_t>(Constants::max_thread_count) + 1);

        // Currently infinitely recursive (see review notes); will not compile under -Werror until fixed.
        REQUIRE(cv.local() == v.local());
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Threading
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("Non Job Non Main Thread Is Rejected", "[core::job::perJobVector]")
    {
        PerJobVector<int> v;

        bool localWasNull = false;
        bool pushed = true;
        bool emplaced = true;

        std::thread other([&]()
            {
                localWasNull = (v.local() == nullptr);
                pushed = v.push_back(1);
                emplaced = v.emplace_back(2);
            });

        other.join();

        REQUIRE(localWasNull);
        REQUIRE_FALSE(pushed);
        REQUIRE_FALSE(emplaced);
        REQUIRE(v.empty());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Concurrent Jobs Across Frames", "[core::job::perJobVector]")
    {
        claimMainThread();

        constexpr uint32_t FrameCount = 3;
        constexpr uint32_t JobCount = 256;
        constexpr uint32_t PerJob = 64;

        PerJobVector<Entry> v;
        std::atomic<uint32_t> failedPushes{ 0 };

        JobScheduler scheduler;

        for (uint32_t frame = 0; frame < FrameCount; ++frame)
        {
            for (uint32_t j = 0; j < JobCount; ++j)
            {
                scheduler.createAndSubmit([&v, &failedPushes, j](Job*)
                    {
                        for (uint32_t s = 0; s < PerJob; ++s)
                        {
                            if (!v.push_back(Entry{ .job = j, .seq = s }))
                            {
                                failedPushes.fetch_add(1, std::memory_order_relaxed);
                            }
                        }
                    }, JobPriority::Normal, nullptr);
            }

            REQUIRE(scheduler.wait());

            // --- Sync point ---

            REQUIRE(failedPushes.load() == 0);
            REQUIRE(v.size() == JobCount * PerJob);

            // Every (job, seq) pair appears exactly once.
            std::vector<uint32_t> seen(JobCount * PerJob, 0u);
            bool inRange = true;

            for (Entry const& e : v.view())
            {
                inRange = inRange && (e.job < JobCount) && (e.seq < PerJob);

                if (inRange)
                {
                    ++seen[e.job * PerJob + e.seq];
                }
            }

            REQUIRE(inRange);
            REQUIRE(std::ranges::all_of(seen, [](uint32_t count) { return count == 1u; }));

            // A job runs to completion on one thread, so its entries must all be in a single
            // slot and in insertion order.
            std::vector<int> owner(JobCount, -1);
            std::vector<uint32_t> nextSeq(JobCount, 0u);
            bool ordered = true;
            int slot = 0;

            for (auto const& vec : v.vectors())
            {
                for (Entry const& e : vec)
                {
                    if (owner[e.job] == -1)
                    {
                        owner[e.job] = slot;
                    }

                    ordered = ordered && (owner[e.job] == slot) && (nextSeq[e.job] == e.seq);
                    ++nextSeq[e.job];
                }

                ++slot;
            }

            REQUIRE(ordered);

            // Drain like a real sync point would; container must be reusable next frame.
            const auto drained = v.drain();
            REQUIRE(drained.size() == JobCount * PerJob);
            REQUIRE(v.empty());
        }
    } LITL_END_TEST_CASE
}