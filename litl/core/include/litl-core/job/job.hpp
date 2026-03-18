#ifndef LITL_ENGINE_JOB_H__
#define LITL_ENGINE_JOB_H__

#include <array>
#include <atomic>
#include <cstdint>

#include "litl-core/alignment.hpp"
#include "litl-core/job/jobPriority.hpp"
#include "litl-core/job/jobHandle.hpp"

namespace LITL::Core
{
    class JobFence;
    struct Job;

    enum JobState
    {
        /// <summary>
        /// Has not yet been submitted to the JobScheduler.
        /// </summary>
        Idle = 0,

        /// <summary>
        /// Submitted to the JobScheduler but not yet been picked up and run by a Worker.
        /// </summary>
        Scheduled = 1,

        /// <summary>
        /// Being run by its owning Worker.
        /// </summary>
        RunningOnOwner = 2,

        /// <summary>
        /// Being run on a thief Worker.
        /// </summary>
        RunningOnThief = 3,

        /// <summary>
        /// Done being run by a Worker.
        /// </summary>
        Complete = 4
    };

    // Note: currently spans 4 cache lines (2 on m-series chips) 
    // can reduce to two (or 1 on m-series) by: reducing buffer to 48 (from 64) and max dependent count to 6 (from 12)
    // time will tell if (a) we need as big of a buffer and/or (b) need as many dependents. can in the future add a BigJob or similar.

    struct alignas(CacheLineSize) Job
    {
        using JobFunc = void(*)(Job* job);
        static constexpr uint32_t JobLocalBufferSize = 64;          // As big as we can get while keeping to two-cache lines on most systems.
        static constexpr uint8_t JobMaxDependentsCount = 8;

        // --- start cache line 0

        /// <summary>
        /// Pointer to the function being executed by this job.
        /// </summary>
        JobFunc func = nullptr;

        /// <summary>
        /// The current version of this job. Allows for job re-use without having to free/reallocate job structs.
        /// The version is incremented each frame and any "out-of-date" versioned jobs are considered stale.
        /// </summary>
        uint32_t version = 0;

        /// <summary>
        /// The priority level of this job.
        /// </summary>
        JobPriority priority = JobPriority::Normal;
        
        /// <summary>
        /// Optional fence that is tracking the progress of this Job.
        /// </summary>
        JobFence* fence = nullptr;

        /// <summary>
        /// Pointer to the user-supplied data be provided to the job at execution.
        /// The caller/creator of the job must ensure that the provided pointer is valid for the lifetime of the Job.
        /// </summary>
        void* data;

        /// <summary>
        /// The number of dependents.
        /// </summary>
        std::atomic<uint32_t> dependentsCount{ 0 };

        /// <summary>
        /// Number of incomplete jobs that this job is still waiting on to finish before it can run.
        /// </summary>
        std::atomic<uint32_t> dependencyCount{ 0 };

        /// <summary>
        /// The state of the job.
        /// </summary>
        std::atomic<JobState> state{ JobState::Idle };

        // --- end cache line 0
        // --- start cache line 1

        /// <summary>
        /// Small buffer of user data stored as part of the Job.
        /// This data is valid for the lifetime of the job.
        /// </summary>
        alignas(CacheLineSize) std::byte localData[JobLocalBufferSize];

        // --- end cache line 1
        // --- start cache line 2

        /// <summary>
        /// All jobs that are dependent on this job to finish before they can run.
        /// </summary>
        alignas(CacheLineSize) std::array<JobHandle, JobMaxDependentsCount> dependents{ };

        // --- end cache line 2

        template<typename T>
        T& getLocalData()
        {
            return reinterpret_cast<T&>(localData);
        }
    };
}

#endif