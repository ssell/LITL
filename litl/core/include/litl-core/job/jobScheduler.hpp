#ifndef LITL_CORE_WORK_SCHEDULER_H__
#define LITL_CORE_WORK_SCHEDULER_H__

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "litl-core/job/job.hpp"
#include "litl-core/job/jobPool.hpp"

namespace LITL::Core
{
    class JobFence;

    /// <summary>
    /// All jobs submitted to this scheduler are expected to start and finish in the same frame.
    /// The wait method should be called at the end of each frame to ensure all jobs are done
    /// </summary>
    class JobScheduler
    {
    public:

        JobScheduler();
        JobScheduler(JobScheduler const&) = delete;
        JobScheduler& operator=(JobScheduler const&) = delete;
        ~JobScheduler();

        /// <summary>
        /// Allocates a Job with the provided optional external data.
        /// 
        /// The returned Job will not run until it is supplied to Submit.
        /// This is to provide the caller time to hook together dependent Jobs if needed.
        /// 
        /// Use createAndSubmit to create a Job and immediately submit it.
        /// </summary>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        /// <returns></returns>
        JobHandle create(Job::JobFunc func, void* externalData = nullptr) noexcept;
        
        /// <summary>
        /// Allocates a Job with the provided Job-local data (which will be copied) and optional external data.
        /// 
        /// The returned Job will not run until it is supplied to Submit.
        /// This is to provide the caller time to hook together dependent Jobs if needed.
        /// 
        /// Use createAndSubmit to create a Job and immediately submit it.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="func"></param>
        /// <param name="jobLocalData"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        /// <returns></returns>
        template<typename T>
        JobHandle create(Job::JobFunc func, T& jobLocalData, void* externalData = nullptr) noexcept
        {
            static_assert(sizeof(T) <= sizeof(Job::localData));

            JobHandle handle = create(func, externalData);
            new (handle.job->localData) T(jobLocalData);

            return handle;
        }

        /// <summary>
        /// Convenience allowing for Job creation via a Lambda.
        /// The lambda is still expected to have the Job::JobFunc signature.
        /// </summary>
        /// <typeparam name="F"></typeparam>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        /// <returns></returns>
        template<typename F> requires (sizeof(F) <= sizeof(Job::localData)) && std::is_trivially_destructible_v<F>
        JobHandle create(F&& func, void* externalData)
        {
            JobHandle handle = create(nullptr);

            // Store the lambda in the job local buffer
            new (handle.job->localData) std::remove_cvref_t<F>(std::forward<F>(func));

            // Set the job function as invoking the lambda.
            handle.job->data = externalData;
            handle.job->func = [](Job* job)
                {
                    void* localData = static_cast<void*>(job->localData);
                    auto& callable = *static_cast<std::remove_cvref_t<F>*>(localData);
                    callable();
                };

            return handle;
        }

        /// <summary>
        /// Allocates a Job with the provided optional external data.
        /// The new Job is immediately submitted to the internal scheduler to be run.
        /// </summary>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        void createAndSubmit(Job::JobFunc func, void* externalData = nullptr, JobPriority priority = JobPriority::Normal) noexcept;

        /// <summary>
        /// Allocates a Job with the provided Job-local data (which will be copied) and optional external data.
        /// The new Job is immediately submitted to the internal scheduler to be run.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="func"></param>
        /// <param name="jobLocalData"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        template<typename T>
        void createAndSubmit(Job::JobFunc func, T& jobLocalData, void* externalData = nullptr, JobPriority priority = JobPriority::Normal) noexcept
        {
            submit(create(func, jobLocalData, externalData), priority);
        }

        /// <summary>
        /// Convenience allowing for Job creation via a Lambda.
        /// The lambda is still expected to have the Job::JobFunc signature.
        /// </summary>
        /// <typeparam name="F"></typeparam>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        template<typename F> requires (sizeof(F) <= sizeof(Job::localData)) && std::is_trivially_destructible_v<F>
        void createAndSubmit(F&& func, void* externalData, JobPriority priority = JobPriority::Normal)
        {
            submit(create(func, externalData), priority);
        }

        /// <summary>
        /// Submits a Job to be run.
        /// There is no guarantee of when the job is run.
        /// </summary>
        /// <param name="job"></param>
        void submit(JobHandle handle, JobPriority priority = JobPriority::Normal) const noexcept;

        /// <summary>
        /// Marks that the specified Job is dependent on another.
        /// 
        /// Note that a job will be automatically submitted when it's dependency has been run.
        /// As such, it is incorrect to list a job with dependencies and then manually submit it.
        /// 
        /// The dependency job (or the job they are dependent on if this is a graph situation) is
        /// the only job that needs to be submitted.
        /// </summary>
        /// <param name="dependent">The job that is dependent on another.</param>
        /// <param name="dependency"></param>
        /// <returns>Can return false if: either Job is null, their versions do not match, or the dependency already has the max number of dependents.</returns>
        bool addDependency(JobHandle dependent, JobHandle dependency) const noexcept;

        /// <summary>
        /// Returns the number of idle or active jobs.
        /// </summary>
        /// <returns></returns>
        uint32_t jobCount() const noexcept;

        /// <summary>
        /// Returns the number of workers.
        /// </summary>
        /// <returns></returns>
        uint32_t workerCount() const noexcept;

        /// <summary>
        /// Returns if the handle is still valid.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        bool valid(JobHandle handle) const noexcept;

        /// <summary>
        /// A blocking sync point on the scheduler.
        /// Waits for all jobs to be completed and then resets the underlying job pool.
        /// </summary>
        /// <returns>True if done waiting without timing out. False if timed out.</returns>
        bool wait(uint32_t timeoutMs = 1000) const noexcept;

    protected:

    private:

        friend class JobFence;

        void workerInternalLoop(uint32_t threadIndex) const;
        std::optional<JobHandle> stealWork(JobPriority priority) const noexcept;
        std::optional<JobHandle> stealAnyWork() const noexcept;
        std::optional<JobHandle> acquireJob(JobPriority priority) const noexcept;
        void run(JobHandle handle) const noexcept;

        struct Impl;
        struct Worker;

        std::unique_ptr<Impl> m_pImpl;

        static thread_local uint32_t t_threadIndex;
    };
}

#endif