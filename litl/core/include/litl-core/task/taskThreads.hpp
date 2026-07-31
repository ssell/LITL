#ifndef LITL_CORE_TASK_THREADS_H__
#define LITL_CORE_TASK_THREADS_H__

#include <condition_variable>
#include <coroutine>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include "litl-core/moveOnlyFunc.hpp"

namespace litl
{
    class TaskThreads final
    {
    public:

        explicit TaskThreads(uint32_t threadCount);
        ~TaskThreads() = default;

        void schedule(std::coroutine_handle<> handle) noexcept;
        

    private:

        std::vector<std::jthread> m_workers;
        std::mutex m_mutex;
        std::condition_variable_any m_conditionVariable;
    };
}

#endif