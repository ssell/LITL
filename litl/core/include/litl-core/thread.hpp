#ifndef LITL_CORE_THREAD_H__
#define LITL_CORE_THREAD_H__

#include <cstdint>
#include <thread>

namespace litl
{
    /// <summary>
    /// Provides basic information about the thread this is called from.
    /// </summary>
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

    /// <summary>
    /// A lightweight thread spinner.
    /// It performs a progressive backoff where early spins PAUSE and later spins YIELD.
    /// 
    /// Note: this is not suitable for longer duration waits as it may waste the CPU. 
    /// Depending on the expected worst-case wait duration, then either a binary_semaphore 
    /// or a condition_variable would be better suited mechanisms.
    /// </summary>
    class ThreadSpin
    {
    public:

        /// <summary>
        /// Resets the internal spin count.
        /// Low count level spins are faster than high count spins.
        /// </summary>
        void reset();

        /// <summary>
        /// Increments the spin count and stalls the thread. 
        /// 
        /// If count is low it perfroms a CPU-level PAUSE.
        /// If count is high it performs an OS-level YIELD hint.
        /// </summary>
        void spin();

    private:

        uint32_t m_count{ 0 };
    };
}

#endif