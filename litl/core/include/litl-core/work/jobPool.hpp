#ifndef LITL_CORE_JOB_POOL_H__
#define LITL_CORE_JOB_POOL_H__

#include <memory>

#include "litl-core/work/job.hpp"

namespace LITL::Core
{
    struct Job;

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
        [[nodiscard]] Job* createJob(uint32_t threadIndex, Job::JobFunc func, void* externalData = nullptr) const noexcept;

        /// <summary>
        /// Marks one job as dependent of another.
        /// The dependent will be automatically submitted once the dependency has been run.
        /// </summary>
        /// <param name="dependent">The job that is dependent on another.</param>
        /// <param name="dependency"></param>
        /// <returns>Can return false if either Job is null or their versions do not match.</returns>
        bool addDependency(Job* dependent, Job* dependency) const noexcept;

        /// <summary>
        /// Reset the global pool and all of the thread-local pools.
        /// Typically called at the end of each frame.
        /// </summary>
        void reset() const noexcept;

        /// <summary>
        /// Returns the current version of the pool.
        /// The version is incremented on each reset.
        /// </summary>
        /// <returns></returns>
        uint32_t version() const noexcept;

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif