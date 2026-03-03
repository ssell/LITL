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

        [[nodiscard]] Job* createJob(JobFunc func, void* userData = nullptr) noexcept;
        void addDependency(Job* job, Job* dependsOn) noexcept;
        void reset() noexcept;

    protected:

    private:

        Job* allocate() noexcept;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        static thread_local uint32_t t_threadIndex;
    };
}

#endif