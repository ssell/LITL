#include "litl-core/task/taskThreads.hpp"
#include "litl-core/constants.hpp"

namespace litl
{
    TaskThreads::TaskThreads(uint32_t threadCount)
    {
        for (uint32_t i = 0u; i < threadCount && i < Constants::max_thread_count; ++i)
        {
            m_workers.emplace_back([this](std::stop_token stop) -> void
            {
                workerLoop(stop);
            });
        }
    }

    void TaskThreads::schedule(std::coroutine_handle<> handle) noexcept
    {
        post([handle]() -> void { handle.resume(); });
    }

    void TaskThreads::post(MoveOnlyFunc<void()> func) noexcept
    {
        {
            // Guard and add the work function to the queue
            std::lock_guard lock(m_mutex);
            m_queue.push_back(std::move(func));
        }

        // Alert waiting workers that there is work to be processed.
        m_conditionVariable.notify_one();
    }

    void TaskThreads::workerLoop(std::stop_token stop) noexcept
    {
        while (true)
        {
            MoveOnlyFunc<void()> workItem;

            {
                std::unique_lock lock(m_mutex);

                // Wait until queue is not empty or stop is requested. 
                // Condition variables can be relatively slow but task threads are for slower, longer running operations anyways.
                m_conditionVariable.wait(lock, stop, [&]() -> bool { return !m_queue.empty(); });

                // If the queue is empty then stop was requested, so exit.
                if (m_queue.empty())
                {
                    return;
                }

                // otherwise, retrieve the work item
                workItem = std::move(m_queue.front());
                m_queue.pop_front();
            }

            workItem();
        }
    }
}