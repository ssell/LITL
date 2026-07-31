#ifndef LITL_CORE_TASK_SYNC_WAIT_H__
#define LITL_CORE_TASK_SYNC_WAIT_H__

#include <semaphore>
#include <thread>

#include "litl-core/task/task.hpp"

namespace litl
{
    /// <summary>
    /// Utility coroutine used by syncWait.
    /// </summary>
    struct SyncWaitCoroutine {
        struct promise_type 
        {
            std::binary_semaphore ready{ 0 };

            SyncWaitCoroutine get_return_object() noexcept
            {
                return SyncWaitCoroutine{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }

            std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            void return_void() noexcept
            {

            }

            void unhandled_exception() noexcept
            {
                std::terminate();
            }

            struct final_awaiter
            {
                static constexpr bool await_ready() noexcept
                {
                    return false;
                }

                void await_suspend(std::coroutine_handle<promise_type> h) noexcept
                {
                    h.promise().ready.release();      // coroutine is fully suspended now
                }

                static constexpr void await_resume() noexcept
                {

                }
            };

            final_awaiter final_suspend() noexcept { return {}; }
        };

        using coroutine_handle = std::coroutine_handle<promise_type>;
        coroutine_handle handle;

        explicit SyncWaitCoroutine(coroutine_handle handle) noexcept : handle(handle) { }
        ~SyncWaitCoroutine() { if (handle) { handle.destroy(); } }
        SyncWaitCoroutine(const SyncWaitCoroutine&) = delete;

        template <class T>
        static SyncWaitCoroutine run(Task<T>& t, TaskStatus<T>& result)
        {
            result = co_await t;
        }
    };

    /// <summary>
    /// Performs a thread-block synchronous wait for the provided Task to complete.
    /// Yes, this defeats the purpose of an asynchronous Task but sometimes is necessary for testing or debugging.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="task"></param>
    /// <returns></returns>
    template<typename T>
    TaskStatus<T> syncWait(Task<T>& task)
    {
        TaskStatus<T> result;
        auto syncWaitCoroutine = SyncWaitCoroutine::run(task, result);

        syncWaitCoroutine.handle.resume();                    // starts run(), which co_awaits t, which starts t
        syncWaitCoroutine.handle.promise().ready.acquire();   // block until final_suspend

        return std::move(result);
    }
}

#endif