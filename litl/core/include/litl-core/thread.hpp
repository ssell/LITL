#ifndef LITL_CORE_THREAD_H__
#define LITL_CORE_THREAD_H__

#include <thread>

namespace LITL::Core
{
    struct ThreadInfo
    {
        std::thread::id id;
        bool mainthread;

        /// <summary>
        /// Retrieves the ThreadInfo for the current thread.
        /// </summary>
        /// <returns></returns>
        static ThreadInfo& get() noexcept;

        /// <summary>
        /// Invoked by the Engine to claim the thread it is instantiated on as the main thread.
        /// </summary>
        static void setMainThread() noexcept;

        /// <summary>
        /// Returns if the current thread is the main thread.
        /// </summary>
        /// <returns></returns>
        static bool isMainThread() noexcept;
    };
}

#endif