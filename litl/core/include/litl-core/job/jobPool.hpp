#ifndef LITL_CORE_JOB_POOL_H__
#define LITL_CORE_JOB_POOL_H__

#include <memory>

#include "litl-core/job/job.hpp"

namespace LITL::Core
{
    struct Job;

    static constexpr uint32_t JobPoolCount = 1024;
    static constexpr uint32_t JobPoolBufferSize = sizeof(Job) * JobPoolCount;

    /// <summary>
    /// A frame-scoped Job pool.
    /// 
    /// All Jobs within this pool are expected to be completed by the time sync is called.
    /// Any outdated JobHandles (whose version does not match the current JobPool version) that 
    /// run, may be doing so on invalid data and the result is undefined.
    /// </summary>
    class JobPool
    {
    public:

        explicit JobPool(uint32_t threadCount = 0);
        JobPool(JobPool const&) = delete;
        JobPool& operator=(JobPool const&) = delete;
        ~JobPool();

        /// <summary>
        /// Allocates a new Job.
        /// </summary>
        /// <param name="threadIndex"></param>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        /// <returns></returns>
        [[nodiscard]] JobHandle createJob(uint32_t threadIndex, Job::JobFunc func, void* externalData = nullptr) const noexcept;

        void release(JobHandle job) const noexcept;

        /// <summary>
        /// Marks one job as dependent of another.
        /// The dependent will be automatically submitted once the dependency has been run.
        /// </summary>
        /// <param name="dependent">The job that is dependent on another.</param>
        /// <param name="dependency"></param>
        /// <returns>Can return false if: either Job is null, their versions do not match, or the dependency already has the max number of dependents.</returns>
        bool addDependency(JobHandle dependent, JobHandle dependency) const noexcept;

        /// <summary>
        /// Reset all of the underlying pools (thread-specific and global) and increments the internal version.
        /// Typically called at the end of each frame.
        /// </summary>
        void sync() const noexcept;

        /// <summary>
        /// Returns the current version of the pool.
        /// The version is incremented on each reset.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t version() const noexcept;

        /// <summary>
        /// Returns the job associated with the handle.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        Job* resolve(JobHandle handle) const noexcept;

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif