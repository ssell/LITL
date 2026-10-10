# LITL Core - Job System

Comprised of the following:

* `Job`
* `JobHandle`
* `Worker`
* `JobScheduler`
* `JobDeque`
* `JobFence`
* `JobPool`
* `JobPriority`

A job is a highly parallelized arbitrary unit of work.

They can not span across frame boundaries and they provide no output - either direct or in the form of callbacks. For all intents and purposes, jobs are fire-and-forget. However, jobs can have dependencies on other jobs.

## Usage

A job can be implemented via a function pointer or a lambda. They can be run with shared external data and/or a local copy of data.

* `Job::data` - simple `void*` that the user must ensure is still valid when the job runs.
* `Job::localData` - fixed 64-byte (`Job::JobLocalBufferSize`) `std::byte` buffer of data copied directly into the job. The copied type must be trivially copyable.

Every `createAndSubmit` overload takes either a `JobPriority` or a `JobFence&` as its second argument. Submitting to a fence uses the fence's priority and lets the caller later block on `JobFence::wait`.

**Function Pointer w/ Shared Data**

```cpp
void jobFoo(Job* job)
{
    auto* data = static_cast<JobData*>(job->data);
    // ...
}

void runJob(JobScheduler& scheduler, JobData& data)
{
    scheduler.createAndSubmit(jobFoo, JobPriority::Normal, &data);
}
```

**Function Pointer w/ Local Data Copy**

```cpp
struct JobData
{
    uint32_t value;
}

void jobFoo(Job* job)
{
    auto& data = job->getLocalData<JobData>();
    // ...
}

void runJob(JobScheduler& scheduler)
{
    JobData data{5};
    scheduler.createAndSubmit(jobFoo, JobPriority::Normal, data, nullptr);
}
```

**Lambda Function**

```cpp
void runJob(JobScheduler& scheduler, JobData& data)
{
    JobFence fence{ &scheduler, JobPriority::High };

    scheduler.createAndSubmit([&data](Job* job)
    {
        // ...
    }, fence, nullptr);

    fence.wait();
}
```

_Note: the lambda must still have the `Job::JobFunc` signature (`void(Job*)`). It may specify shared data (`nullptr` in the example), but not local data, because the lambda closure itself is stored within the local data buffer. The closure must therefore fit in 64 bytes and be trivially destructible (it is never destroyed)._

## Scheduling

Jobs are run by Workers and are processed in accordance to their priority: High, Normal, Low.

The scheduler creates `min(max(2, CPU concurrency), 32)` workers (`Constants::max_thread_count` is 32). Worker 0 is the main thread: it has no thread loop of its own and only executes jobs while the main thread is blocked in `JobFence::wait` or `JobScheduler::wait`. Every other worker runs on its own thread. The last worker is dedicated to High priority jobs and ignores Normal and Low work, which avoids edge-cases where every worker is busy with slower, low priority work while high priority jobs pile up.

When a job is submitted it is added to the deque of the thread-local worker. Each worker has one deque per priority level. When a worker runs, it walks the priority levels from highest to lowest and, at each level, first pops from its own deque and then tries to steal from a random other worker before moving to the next level:

```
Worker Run:
    Pop High priority job from own deque. If none, steal High priority job from a random worker.
    If still none: pop Normal priority job from own deque. If none, steal Normal priority job.
    If still none: pop Low priority job from own deque. If none, steal Low priority job.
    (The dedicated High worker only checks the High level.)

    If a job was found, run it.
    Otherwise sleep for up to 50 microseconds or until awoken by the scheduler.
```

This means a local Normal job is never run while a High job is available to steal.

Steals are done to a randomly selected Worker in order to avoid contention on the top (tail) of the deques. While a cold-start may see a disproportionate number of Jobs belonging to a single Worker (due to a main thread kicking things off), the workload quickly spreads out over all Workers as Jobs are stolen. When those Jobs are stolen, any further Jobs that they spawn directly or indirectly (via dependents) will be submitted to the thief Worker. Thus over a short period of time the optimal case for scanning, if contention is ignored, is no longer valid.

If a job was successfully popped or stolen, then:

```
    If job is valid (non-null func, version matches scheduler):
        Execute job func

    Foreach dependent:
        Decrement dependent dependency count.
        If dependent dependency count is now 0, submit dependent to scheduler (at this job's priority).

    If job is contained in a Fence, alert the fence that the job has been run.

    Decrement scheduler overall job count.
```

Dependents and fences are processed even if the job is no longer valid, so that a stale job can never leave a fence or dependent waiting forever.

## Synchronization

Job synchronization is accomplished via dependencies (Job B is dependent on Job A, etc.), fences (block until specific jobs are complete), or overall scheduler wait/sync (at frame end).

When a fence or the scheduler is waiting, it performs work on the waiting thread. This helps increase job throughput while also avoiding deadlocks.

* `JobFence::wait` - blocks until the individual jobs added to the fence are complete.
* `JobScheduler::wait` - blocks until _all_ jobs are complete.

Both the `JobFence` and `JobScheduler` use a progressive backoff via `litl::ThreadSpin` if/when a steal fails and there are still jobs in progress.

## Deque

The underlying `JobDeque` is an implementation of the Chase-Lev work-stealing deque:

https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf

Jobs are processed in a LIFO manner by their own thread (`pop`), and FIFO by other threads (`steal`).

## Pooling

Jobs are pooled in thread-local buffers and a global buffer.

The thread-local buffers can hold 1024 jobs each. Once a local buffer is full, additional allocations overflow into the global job buffer. The global buffer uses paged memory and generally does not shrink.

Local pools are more efficient than the global pool as allocation simply increments the buffer offset. The global pool also increments a buffer offset, but one that is stored in an atomic, and if necessary it must allocate another memory page. While fast, it is still slower than a local pool.

When the work scheduler syncs (`JobScheduler::wait`), it resets all job pools. This is done efficiently by simply resetting the current offsets into each buffer. Because no data is actually cleared during a reset, it is imperative that a `JobHandle` is used as opposed to a raw `Job` pointer. A raw pointer can point to out-of-date memory, whereas a handle is trivially validated via `JobScheduler::valid`.

