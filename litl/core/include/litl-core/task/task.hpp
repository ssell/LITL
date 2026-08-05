#ifndef LITL_CORE_TASK_TASK_H__
#define LITL_CORE_TASK_TASK_H__

#include <atomic>
#include <coroutine>
#include <optional>

#include "litl-core/task/taskStatus.hpp"

namespace litl
{
    /// <summary>
    /// A coroutine based async Task. Intended for async operations that span multiple frames.
    /// This is in contrast to Jobs were are for intraframe async operations.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    template <typename T>
    struct Task
    {
        /// <summary>
        /// Required internal structure that makes this a valid C++ coroutine.
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
                    auto continuation = handle.promise().continuation;

                    handle.promise().finished.store(true, std::memory_order_release);

                    return continuation;
                }

                void await_resume() noexcept
                {

                }
            };

            /// <summary>
            /// State and execution pointer of the suspended outer coroutine.
            /// </summary>
            std::coroutine_handle<> continuation = std::noop_coroutine();

            /// <summary>
            /// The stored final value of the Task.
            /// </summary>
            TaskStatus<T> value;

            /// <summary>
            /// Is this Task finished running (regardless of value)?
            /// </summary>
            std::atomic<bool> finished{ false };

            /// <summary>
            /// Called first. Returns the outer coroutine object (Task or Generator).
            /// </summary>
            Task get_return_object()
            {
                return Task(std::coroutine_handle<promise_type>::from_promise(*this));
            }

            /// <summary>
            /// Called before the coroutine body runs and returns lazy/eager on if it should be run immediately. We choose lazy.
            /// </summary>
            std::suspend_always initial_suspend() noexcept
            {
                value.status = TaskStatusType::Running;
                return {};
            }

            /// <summary>
            /// Called after the coroutine finishes and must return an awaitable. We return our custom final_awaiter.
            /// </summary>
            /// <returns></returns>
            final_awaiter final_suspend() noexcept
            {
                value.status = (value.status != TaskStatusType::Error ? TaskStatusType::Complete : TaskStatusType::Error);
                return {};
            }

            /// <summary>
            /// Called when the coroutine is yielded (co_yield) and stores the yielded value.
            /// </summary>
            /// <param name="v"></param>
            /// <returns></returns>
            std::suspend_always yield_value(T v) noexcept
            {
                value.value = std::move(v);
                return {};
            }

            /// <summary>
            /// Required if the coroutine returns a value via co_return.
            /// </summary>
            void return_value(T v)
            {
                value.value = std::move(v);
            }

            /// <summary>
            /// Called if an exception escapes the coroutine body.
            /// We compile with exceptions disabled, so that is not a concern for us.
            /// </summary>
            void unhandled_exception()
            {
                value.status = TaskStatusType::Error;
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
        /// Called when the coroutine resumes (when await_ready returns true).
        /// The return type of this method dictates the value value of the co_await.
        /// </summary>
        /// <returns></returns>
        TaskStatus<T> await_resume()
        {
            return std::move(handle.promise().value);
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

        /// <summary>
        /// The current value of the task, including its status.
        /// </summary>
        /// <returns></returns>
        TaskStatus<T> value() const noexcept
        {
            return handle.promise().value;
        }
    };
}

#endif