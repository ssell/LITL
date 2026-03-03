#ifndef LITL_ENGINE_JOB_H__
#define LITL_ENGINE_JOB_H__

#include <atomic>
#include <cstdint>

namespace LITL::Core
{
    struct Job
    {
        using JobFunc = void(*)(Job* job, uint32_t threadIndex);

        /// <summary>
        /// Pointer to the function being executed by this job.
        /// </summary>
        JobFunc func = nullptr;

        /// <summary>
        /// Pointer to the user-supplied data be provided to the job at execution.
        /// The caller/creator of the job must ensure that the provided pointer is valid for the lifetime of the Job.
        /// </summary>
        void* data;

        /// <summary>
        /// Small buffer of user data stored as part of the Job.
        /// This data is valid for the lifetime of the job.
        /// </summary>
        std::byte localData[64];

        /// <summary>
        /// Number of incomplete jobs that this job is still waiting on to finish before it can run.
        /// </summary>
        std::atomic<uint32_t> dependencyCount{ 0 };

        /// <summary>
        /// The current version of this job. Allows for job re-use without having to free/reallocate job structs.
        /// The version is incremented each frame and any "out-of-date" versioned jobs are considered stale.
        /// </summary>
        uint32_t version = 0;

        /// <summary>
        /// All jobs that are dependent on this job to finish before they can run.
        /// </summary>
        std::vector<Job*> dependents;

        template<typename T>
        T& getLocalData()
        {
            return reinterpret_cast<T*>(localData);
        }
    };
}

#endif