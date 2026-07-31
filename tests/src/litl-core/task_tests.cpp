#include "tests.hpp"
#include "litl-core/thread.hpp"

#include <coroutine>
#include <filesystem>
#include <optional>
#include <semaphore>
#include <thread>

/**
 * I'll be honest.
 * 
 * I don't understand coroutines in C++. But I am pretty sure I want them for my implementation of multi-frame Tasks.
 * So, here is me trying to learn them.
 */

namespace litl::tests
{
    namespace
    {
        // ---------------------------------------------------------------------------------
        // Generator
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// The simplest type of coroutine - a generator. Make use of only co_yield and has no actual async.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        template<typename T>
        struct Generator
        {
            /// <summary>
            /// This is mandated by C++ and provides the required methods needed by the compiler.
            /// </summary>
            struct promise_type
            {
                T value{};

                /// <summary>
                /// Called first. Returns the outer coroutine object (Task or Generator).
                /// </summary>
                Generator get_return_object() 
                { 
                    return Generator{ 
                        std::coroutine_handle<promise_type>::from_promise(*this) 
                    }; 
                }

                /// <summary>
                /// Called before the coroutine body runs and returns an awaitable. We choose the default lazy awaitable.
                /// </summary>
                std::suspend_always initial_suspend() noexcept 
                { 
                    return {}; 
                }

                /// <summary>
                /// Called after the coroutine finishes and must return an awaitable. We choose the default lazy awaitable.
                /// </summary>
                std::suspend_always final_suspend() noexcept 
                { 
                    return {}; 
                }

                /// <summary>
                /// This is called at the end and is required if the coroutine ends with co_routine or falls off the end.
                /// If the coroutine was returning a value, we would use return_value instead.
                /// </summary>
                void return_void() {}

                /// <summary>
                /// Required if the coroutine used co_yield. Responsible for storing the yielded value and must return an awaitable.
                /// </summary>
                std::suspend_always yield_value(T v)
                {
                    value = std::move(v);
                    return {};
                }

                /// <summary>
                /// Called if an exception escapes the coroutine body.
                /// We compile with exceptions disabled, so that is not a concern for us.
                /// </summary>
                void unhandled_exception() 
                {
                
                }
            };

            using coroutine_handle = std::coroutine_handle<promise_type>;

            explicit Generator(coroutine_handle handle) : handle(handle) {}
            ~Generator() { if (handle) { handle.destroy(); } }
            Generator(Generator&& other) noexcept : handle(std::exchange(other.handle, {})) {}
            Generator(Generator const&) = delete;

            bool next()
            {
                if (!handle || handle.done())
                {
                    return false;
                }

                handle.resume();

                return !handle.done();
            }

            T& value()
            {
                return handle.promise().value;
            }

            coroutine_handle handle;
        };

        Generator<uint32_t> summation(uint32_t start, uint32_t end)
        {
            uint32_t value = 0u;

            for (uint32_t i = start; i <= end; ++i)
            {
                value += i;
                co_yield value;
            }
        }

        // ---------------------------------------------------------------------------------
        // Task
        // ---------------------------------------------------------------------------------

        template <typename T>
        struct Task
        {
            /// <summary>
            /// Again, the required internal structure that makes this a valid C++ coroutine.
            /// </summary>
            struct promise_type
            {
                struct final_awaiter
                {
                    bool await_ready() noexcept
                    {
                        return false;
                    }

                    std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> handle) noexcept
                    {
                        return handle.promise().continuation;
                    }

                    void await_resume() noexcept
                    {

                    }
                };

                std::optional<T> result;
                std::coroutine_handle<> continuation = std::noop_coroutine();

                /// <summary>
                /// Called first. Returns the outer coroutine object (Task or Generator).
                /// </summary>
                Task get_return_object()
                {
                    return Task{
                        std::coroutine_handle<promise_type>::from_promise(*this)
                    };
                }

                /// <summary>
                /// Called before the coroutine body runs and returns lazy/eager on if it should be run immediately. We choose lazy.
                /// </summary>
                std::suspend_always initial_suspend() noexcept
                {
                    return {};
                }

                /// <summary>
                /// Called after the coroutine finishes and must return an awaitable. We return our custom final_awaiter.
                /// </summary>
                /// <returns></returns>
                final_awaiter final_suspend() noexcept
                {
                    return {};
                }

                /// <summary>
                /// Required if the coroutine returns a value via co_return.
                /// </summary>
                void return_value(T v)
                {
                    result = std::move(v);
                }

                /// <summary>
                /// Called if an exception escapes the coroutine body.
                /// We compile with exceptions disabled, so that is not a concern for us.
                /// </summary>
                void unhandled_exception()
                {

                }
            };

            using coroutine_handle = std::coroutine_handle<promise_type>;
            coroutine_handle handle;

            explicit Task(coroutine_handle handle) : handle(handle) {}
            ~Task() { if (handle) { handle.destroy(); } }
            Task(Task&& other) noexcept : handle(std::exchange(other.handle, {})) {}
            Task(Task const&) = delete;

            /// <summary>
            /// Required for an awaitable coroutine. Must return a bool.
            /// If true, the coroutine does not suspend and immediately calls await_resume.
            /// If false, the coroutine suspends and calls await_suspend.
            /// </summary>
            bool await_ready() const noexcept
            {
                return !handle || handle.done();
            }

            /// <summary>
            /// Called when the coroutine resumts (when await_ready returns true).
            /// The return type of this method dictates the result value of the co_await.
            /// </summary>
            /// <returns></returns>
            T await_resume()
            {
                return std::move(*handle.promise().result);
            }

            /// <summary>
            /// Called if await_ready returns false.
            /// This comes in three flavors based on return type:
            /// 
            ///     void: Suspends and returns control to the caller.
            ///     bool: If true suspends, if false returns execution immediately.
            ///     std::coroutine_handle: Suspends the current coroutine and immediately executes the returned coroutine handle (symmetric transfer).
            /// </summary>
            std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting) noexcept
            {
                handle.promise().continuation = awaiting;
                return handle;
            }
        };

        /// <summary>
        /// Not really a thread pool, but good enough to demonstrate the coroutine being run on a separate thread.
        /// </summary>
        struct TestThreadPool
        {
            void enqueue(auto func) const
            {
                std::jthread thread([func]() { func(); });
            }
        };

        /// <summary>
        /// Not an awaitable itself, but operates on one.
        /// </summary>
        struct ResumeOnThreadPool
        {
            TestThreadPool pool{};

            /// <summary>
            /// Returns false so that the coroutine suspends and we immediately invoke await_suspend.
            /// By hardcoding false, we never enter await_resume.
            /// </summary>
            bool await_ready() const noexcept
            {
                return false;
            }

            /// <summary>
            /// 
            /// </summary>
            /// <param name="handle"></param>
            void await_suspend(std::coroutine_handle<> handle) const
            {
                pool.enqueue([handle]() -> void {
                    handle.resume();
                });
            }

            /// <summary>
            /// Unused for this.
            /// </summary>
            void await_resume() const noexcept
            {

            }
        };

        /// <summary>
        /// Draining queue intended to run on the main thread so that a coroutine can hop back over to it once it is done with the worker thread.
        /// ... todo ... I still need to fully go through this ...
        /// </summary>
        class MainThreadQueue {
            std::mutex m;
            std::vector<std::coroutine_handle<>> pending, draining;
            std::thread::id owner = std::this_thread::get_id();
        public:
            bool is_current() const noexcept { return std::this_thread::get_id() == owner; }

            void post(std::coroutine_handle<> h) {
                std::scoped_lock lk{ m };
                pending.push_back(h);
            }

            void drain() {                       // call from the tick loop
                {
                    std::scoped_lock lk{ m };
                    std::swap(pending, draining);   // snapshot, so resumes can re-post safely
                }
                for (auto h : draining) h.resume();
                draining.clear();
            }
        };

        /// <summary>
        /// Returns the main thread queue. Need to call this from whatever the main thread is prior to it being used by the ResumeOnMain.
        /// </summary>
        /// <returns></returns>
        static MainThreadQueue& GetMainThreadQueue()
        {
            static MainThreadQueue mtq{};
            return mtq;
        }

        /// <summary>
        /// Like ResumeOnThreadPool but for the main thread.
        /// ... todo ... I still need to fully go through this ...
        /// </summary>
        struct ResumeOnMain {
            MainThreadQueue& q;
            bool await_ready() const noexcept { return q.is_current(); }  // already there → skip
            void await_suspend(std::coroutine_handle<> h) const { GetMainThreadQueue().post(h); }
            void await_resume() const noexcept {}
        };

        static ThreadInfo workerThreadInfo{};

        /// <summary>
        /// Retrieves the file size from a worker thread and then hops back to the main/calling thread and returns the value.
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        Task<size_t> loadFileSize(std::string path)
        {
            co_await ResumeOnThreadPool{};
            workerThreadInfo = ThreadInfo::get();
            size_t size = static_cast<size_t>(std::filesystem::file_size(path));
            co_await ResumeOnMain{ GetMainThreadQueue() };
            co_return size;
        }

        namespace detail {
            /// <summary>
            /// A helper coroutine for implementing the synchronous wait used by the test.
            /// It kinda defeats the purpose of an async coroutine as it blocks on the calling thread but it is useful for testing.
            /// ... todo ... I still need to fully go through this ...
            /// </summary>
            struct SyncWaitCoro {
                struct promise_type {
                    std::binary_semaphore ready{ 0 };

                    SyncWaitCoro get_return_object() noexcept {
                        return SyncWaitCoro{ std::coroutine_handle<promise_type>::from_promise(*this) };
                    }
                    std::suspend_always initial_suspend() noexcept { return {}; }
                    void return_void() noexcept {}
                    void unhandled_exception() noexcept { std::terminate(); }  // body catches everything

                    struct final_awaiter {
                        static constexpr bool await_ready() noexcept { return false; }
                        void await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                            h.promise().ready.release();      // coroutine is fully suspended now
                        }
                        static constexpr void await_resume() noexcept {}
                    };
                    final_awaiter final_suspend() noexcept { return {}; }
                };
                using handle = std::coroutine_handle<promise_type>;
                handle h;
                explicit SyncWaitCoro(handle h) noexcept : h(h) {}
                ~SyncWaitCoro() { if (h) h.destroy(); }
                SyncWaitCoro(const SyncWaitCoro&) = delete;   // lives on sync_wait's stack, never moves
            };

            template <class T>
            SyncWaitCoro run(Task<T>& t, std::optional<T>& value, std::exception_ptr& err) {
                value = co_await t;
            }

        } // namespace detail

        /// <summary>
        /// Forces a wait for the async coroutine to finish.
        /// Defeats the purpose of the async coroutine but is needed for testing.
        /// ... todo ... I still need to fully go through this ...
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="t"></param>
        /// <returns></returns>
        template <class T>
        T sync_wait(Task<T> t) {
            std::optional<T> value;
            std::exception_ptr err;

            auto coro = detail::run(t, value, err);
            coro.h.resume();                    // starts run(), which co_awaits t, which starts t
            coro.h.promise().ready.acquire();   // block until final_suspend
            return std::move(*value);
        }
    }

    // -------------------------------------------------------------------------------------
    // Tests
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("Summation", "[core::tasks]")
    {
        auto sum1to10 = summation(1, 10);

        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 1u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 3u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 6u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 10u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 15u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 21u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 28u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 36u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 45u);
        REQUIRE(sum1to10.next() == true); REQUIRE(sum1to10.value() == 55u);
        REQUIRE(sum1to10.next() == false);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Task", "[core::tasks]")
    {
        auto originalThread = ThreadInfo::get();
        size_t size = sync_wait(loadFileSize("litl-tests.exe"));
        auto resolvingThread = ThreadInfo::get();
        auto workerThread = workerThreadInfo;

        REQUIRE(size > 0ull);
        REQUIRE(originalThread.index == resolvingThread.index);
        REQUIRE(originalThread.index != workerThread.index);
    } LITL_END_TEST_CASE
}