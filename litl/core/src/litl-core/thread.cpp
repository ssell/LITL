#include <atomic>
#include <thread>

#include "litl-core/thread.hpp"

namespace litl
{
    // -------------------------------------------------------------------------------------
    // ThreadInfo
    // -------------------------------------------------------------------------------------

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

    // -------------------------------------------------------------------------------------
    // ThreadSpin
    // -------------------------------------------------------------------------------------

    void ThreadSpin::reset()
    {
        m_count = 0;
    }

    void ThreadSpin::spin()
    {
        if (m_count++ < 64)
        {
            // CPU-level PAUSE for early spins
#if defined(__x86_64__) || defined(_M_X64)
            _mm_pause();
#elif defined(__aarch64__)
            __yield();
#endif
        }
        else
        {
            // OS-level YIELD hint
            std::this_thread::yield();
        }
    }
}