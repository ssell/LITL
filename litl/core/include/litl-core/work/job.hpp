#ifndef LITL_ENGINE_JOB_H__
#define LITL_ENGINE_JOB_H__

#include <atomic>
#include <cstdint>

namespace LITL::Core
{
    struct Job
    {
        using JobFunc = void(*)(void* userdata, uint32_t threadIndex);

        /// <summary>
        /// Pointer to the function being executed by this job.
        /// </summary>
        JobFunc func = nullptr;

        /// <summary>
        /// Pointer to the user-supplied data be provided to the job at execution.
        /// </summary>
        void* userData = nullptr;

        /// <summary>
        /// Number of jobs that this job is still waiting on to finish before it can run.
        /// </summary>
        std::atomic<uint32_t> dependentOnCount{ 0 };

        /// <summary>
        /// All jobs that are dependent on this job to finish before they can run.
        /// </summary>
        std::vector<Job*> dependencies;

        /// <summary>
        /// The current version of this job. Allows for job re-use without having to free/reallocate job structs.
        /// The version is incremented each frame and any "out-of-date" versioned jobs are considered stale.
        /// </summary>
        uint32_t version = 0;
    };
}

#endif