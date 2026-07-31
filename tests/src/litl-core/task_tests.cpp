#include <coroutine>
#include <filesystem>
#include <optional>
#include <semaphore>
#include <thread>


#include "tests.hpp"
#include "litl-core/task/task.hpp"
#include "litl-core/task/syncWait.hpp"
#include "litl-core/thread.hpp"

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
        Task<size_t> loadFileSize(std::string path)
        {
            /*
            co_await ResumeOnThreadPool{};
            workerThreadInfo = ThreadInfo::get();
            size_t size = static_cast<size_t>(std::filesystem::file_size(path));
            co_await ResumeOnMain{ GetMainThreadQueue() };
            co_return size;
            */

            size_t size = static_cast<size_t>(std::filesystem::file_size(path));
            co_return size;
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
        const auto originalThread = ThreadInfo::get();
        
        {
            Task<size_t> loadFileSizeTask = loadFileSize("litl-tests.exe");
            TaskStatus<size_t> taskResult = syncWait(loadFileSizeTask);
            REQUIRE(taskResult.result.has_value());
            REQUIRE(taskResult.result.value() > 0ull);
        }

        const auto resolvingThread = ThreadInfo::get();           // make sure we are returning back to our original thread
        const auto workerThread = workerThreadInfo;               // make sure the work itself was done on a separate thread

        REQUIRE(originalThread.index == resolvingThread.index);
        REQUIRE(originalThread.index != workerThread.index);
    } LITL_END_TEST_CASE
}