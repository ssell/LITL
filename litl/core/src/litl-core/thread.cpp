#include <atomic>
#include "litl-core/thread.hpp"

namespace LITL::Core
{
    ThreadInfo& ThreadInfo::get() noexcept
    {
        static thread_local ThreadInfo threadInfo{
            .id = std::this_thread::get_id(),
            .mainthread = false
        };

        return threadInfo;
    }

    void ThreadInfo::setMainThread() noexcept
    {
        static std::atomic<bool> IsMainThreadClaimed{ false };

        if (IsMainThreadClaimed.exchange(true) == false)
        {
            ThreadInfo::get().mainthread = true;
        }
    }

    bool ThreadInfo::isMainThread() noexcept
    {
        return ThreadInfo::get().mainthread;
    }
}