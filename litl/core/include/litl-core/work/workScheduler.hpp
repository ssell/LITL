#ifndef LITL_CORE_WORK_SCHEDULER_H__
#define LITL_CORE_WORK_SCHEDULER_H__

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "litl-core/work/job.hpp"
#include "litl-core/work/jobPool.hpp"

namespace LITL::Core
{
    class WorkFence;

    class WorkScheduler
    {
    public:

        explicit WorkScheduler(uint32_t threadCount = 0);
        WorkScheduler(WorkScheduler const&) = delete;
        WorkScheduler& operator=(WorkScheduler const&) = delete;
        ~WorkScheduler();

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
        Job* create(Job::JobFunc func, void* externalData = nullptr) noexcept;
        
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
        Job* create(Job::JobFunc func, T& jobLocalData, void* externalData = nullptr) noexcept
        {
            static_assert(sizeof(T) <= sizeof(Job::JobLocalBufferSize));
            static_assert(alignof(T) <= alignof(decltype(Job::JobLocalBufferSize)));

            Job* job = create(func, externalData);
            new (job->localData) T(jobLocalData);

            return job;
        }

        /// <summary>
        /// Convenience allowing for Job creation via a Lambda.
        /// The lambda is still expected to have the Job::JobFunc signature.
        /// </summary>
        /// <typeparam name="F"></typeparam>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        /// <returns></returns>
        template<typename F> requires (sizeof(F) <= Job::JobLocalBufferSize) && std::is_trivially_destructible_v<F>
        Job* create(F&& func, void* externalData = nullptr)
        {
            Job* job = create(nullptr);

            // Store the lambda in the job local buffer
            new (job->localData) std::decay_t<F>(std::forward<F>(func));

            // Set the job function as invoking the lambda.
            job->data = externalData;
            job->func = [](Job* job, uint32_t threadIndex)
                {
                    auto& callable = *static_cast<std::decay_t<F>*>(job->localData);
                    callable(job, threadIndex);
                };

            return job;
        }

        /// <summary>
        /// Allocates a Job with the provided optional external data.
        /// The new Job is immediately submitted to the internal scheduler to be run.
        /// </summary>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        void createAndSubmit(Job::JobFunc func, void* externalData = nullptr) noexcept;

        /// <summary>
        /// Allocates a Job with the provided Job-local data (which will be copied) and optional external data.
        /// The new Job is immediately submitted to the internal scheduler to be run.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="func"></param>
        /// <param name="jobLocalData"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        template<typename T>
        void createAndSubmit(Job::JobFunc func, T& jobLocalData, void* externalData = nullptr) noexcept
        {
            submit(createAndSubmit(func, jobLocalData, externalData));
        }

        /// <summary>
        /// Convenience allowing for Job creation via a Lambda.
        /// The lambda is still expected to have the Job::JobFunc signature.
        /// </summary>
        /// <typeparam name="F"></typeparam>
        /// <param name="func"></param>
        /// <param name="externalData">Pointer to externally provided data. Caller is responsible for ensuring the pointer is valid for the lifetime of the Job.</param>
        template<typename F> requires (sizeof(F) <= Job::JobLocalBufferSize) && std::is_trivially_destructible_v<F>
        void createAndSubmit(F&& func, void* externalData = nullptr)
        {
            submit(create(func, externalData));
        }

        /// <summary>
        /// Submits a Job to be run.
        /// There is no guarantee of when the job is run.
        /// </summary>
        /// <param name="job"></param>
        void submit(Job* job) const noexcept;

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
        bool addDependency(Job* dependent, Job* dependency) const noexcept;

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

    protected:

    private:

        friend class WorkFence;

        void workerInternalLoop(uint32_t threadIndex) const;
        std::optional<Job*> stealWork(std::minstd_rand& prng) const noexcept;
        std::optional<Job*> acquireJob() const noexcept;
        void run(Job* job) const noexcept;

        struct Impl;
        struct Worker;

        std::unique_ptr<Impl> m_pImpl;

        static thread_local uint32_t t_threadIndex;
    };
}

#endif