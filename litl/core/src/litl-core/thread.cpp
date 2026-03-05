#include <atomic>
#include "litl-core/thread.hpp"

namespace LITL::Core
{
    static ThreadInfo& threadInfo() noexcept
    {
        static thread_local ThreadInfo threadInfo{
            .id = std::this_thread::get_id(),
            .isMainThread = false
        };

        return threadInfo;
    }

    static void setMainThread() noexcept
    {
        static std::atomic<bool> IsMainThreadClaimed{ false };

        if (IsMainThreadClaimed.exchange(true) == false)
        {
            threadInfo().isMainThread = true;
        }
    }

    static bool isMainThread() noexcept
    {
        return threadInfo().isMainThread;
    }
}