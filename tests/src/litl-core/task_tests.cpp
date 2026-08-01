#include <filesystem>

#include "tests.hpp"
#include "litl-core/task.hpp"
#include "litl-core/thread.hpp"

namespace litl::tests
{
    namespace
    {
        TaskThreadPool& GetTestTaskThreadPool() noexcept
        {
            static TaskThreadPool threadPool(1u);
            return threadPool;
        }

        static ThreadInfo workerThreadInfo{};

        /// <summary>
        /// Retrieves the file size from a worker thread and then hops back to the main/calling thread and returns the value.
        /// </summary>
        Task<size_t> testLoadFileSize(std::string path)
        {
            co_await ResumeTaskOnWorkerThread{ GetTestTaskThreadPool() };   // Go from the main thread to the worker thread.

            workerThreadInfo = ThreadInfo::get();
            size_t size = static_cast<size_t>(std::filesystem::file_size(path));

            co_await ResumeTaskOnMainThread{};                              // Go from the worker thread back to the main thread.
            co_return size;
        }
    }

    // -------------------------------------------------------------------------------------
    // Tests
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("Task", "[core::tasks]")
    {
        TaskThreadQueue::RegisterMainThreadQueue();                 // Normally called by the engine, but need to do it ourselves in tests.
        const auto originalThread = ThreadInfo::get();
        
        {
            // Create our Task. Note it does not run until it is directed to (either via a co_await or resume).
            Task<size_t> loadFileSizeTask = testLoadFileSize("litl-tests.exe");
            TaskStatus<size_t> taskResult{};

            // Normally the Task would process automatically by the Engine since it drains the main thread TaskThreadQueue.
            // However, we do not have that behavior in the test suite. So we need to manually execute the Task and then
            // keep calling drain on the main thread queue until the Task is done.
            auto syncWaitCoroutine = SyncWaitCoroutine::run(loadFileSizeTask, taskResult);

            syncWaitCoroutine.handle.resume();

            while (!syncWaitCoroutine.handle.done())
            {
                TaskThreadQueue::GetMainThreadQueue().drain();
            }

            // Make sure the task completed successfully with a non-zero file size.
            REQUIRE(taskResult.result.has_value());
            REQUIRE(taskResult.result.value() > 0ull);
        }

        // Make sure the task performed work on a separate worker thread and returned back to the main thread upon completion.
        const auto resolvingThread = ThreadInfo::get();
        const auto workerThread = workerThreadInfo;

        REQUIRE(originalThread.index == resolvingThread.index);
        REQUIRE(originalThread.index != workerThread.index);
    } LITL_END_TEST_CASE
}