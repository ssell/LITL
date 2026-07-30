#ifndef LITL_CORE_TASK_H__
#define LITL_CORE_TASK_H__

#include <coroutine>
#include <optional>

namespace litl
{
    template<typename T>
    struct Task
    {
        struct promise_type
        {
            std::optional<T> result = std::nullopt;
            std::coroutine_handle<> continuation = std::noop_coroutine();

            Task get_return_object()
            {
                return Task
                {
                    std::coroutine_handle<promise_type>::from_promise(*this)
                };
            }

            std::suspend_always initial_suspend() noexcept
            {
                return {};
            }

            void return_value(T value)
            {
                result = std::move(value);
            }

            void unhandled_exception()
            {
                // ... todo ...
            }

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

            final_awaiter final_suspend() noexcept
            {
                return {};
            }
        };

        std::coroutine_handle<promise_type> handle;

        explicit Task(std::coroutine_handle<promise_type> handle) : handle(handle)
        {

        }

        ~Task()
        {
            if (handle)
            {
                handle.destroy();
            }
        }

        Task(Task&& other) noexcept : handle(std::exchange(other.handle, {}))
        {

        }

        Task(Task const& other) = delete;

        bool await_ready() const noexcept
        {
            return !handle || handle.done();
        }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> awaiting) noexcept
        {
            handle.promise().continuation = awaiting;
            return handle;
        }

        T await_resume()
        {
            return std::move(*handle.promise().result);
        }
    };
}

#endif