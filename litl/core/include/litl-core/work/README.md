# Work System

Comprised of the following:

* `Job`
* `JobHandle`
* `Worker`
* `WorkScheduler`
* `WorkDeque`
* `WorkFence`

A job is a highly parallelized arbitrary unit of work.

They can not span across frame boundaries and they provide no output - either direct or in the form of callbacks. For all intents and purposes, jobs are fire-and-forget. However, jobs can have dependencies on other jobs.

## Usage

A job can be implemented via a function pointer or a lambda. They can be run with shared external data and/or a local copy of data.

* `Job::data` - simple `void*` that the user must ensure is still valid when the job runs.
* `Job::localData` - fixed `std::byte` buffer of data copied directly into the job.

**Function Pointer w/ Shared Data**

```cpp
void jobFoo(Job* job)
{
    auto* data = static_cast<JobData*>(job->data);
    // ...
}

void runJob(WorkScheduler& scheduler, JobData& data)
{
    scheduler.createAndSubmit(jobFoo, data);
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

void runJob(WorkScheduler& scheduler)
{
    JobData data{5};
    scheduler.createAndSubmit(jobFoo, data, nullptr);
}
```

**Lambda Function**

```cpp
void runJob(WorkScheduler& scheduler, JobData& data)
{
    scheduler.createAndSubmit([&data]()
    {
        // ...
    }, nullptr);
}
```

_Note: a lambda function may specify shared data (`nullptr` in the example), but not local data. The lambda closure itself is stored within the local data buffer._

## Scheduling

Jobs are run by Workers and are processed in accordance to their priority: High, Normal, Low.

By default there are `max(1, CPU Concurreny Limit - 1)` workers, each running in their own thread. Additionally, there is a single dedicated worker for High priority jobs. The high priority worker is to avoid edge-cases where all workers are busy processing slower, low priority work while high priority jobs pile up.

When a job is submitted it is added to the deque of the thread-local worker. When a worker runs it pops a job from its deque to execute. If the worker-specific deque is empty then it attempts to steal a job from a random other worker. Jobs are processed in accordance to their priority, with each priority level given its own deque on each worker.

```
Worker Run:
    Pop job from High priority deque. If job retrieved, run it. Else,
    Pop job from Normal priority deque. If job retrieved run it. Else,
    Pop job from Low priority deque. If job retrieved run it. Else,

    No thread-local jobs found.

    Steal High priority job from other worker. If job stolen, run it. Else,
    Steal Normal priority job from other worker. If job stolen, run it. Else,
    Steal Low priority job from other worker. If job stolen, run it.

    No jobs stolen, then sleep for 50 microseconds or until awoken by scheduler.
```

If a job was successfully popped or stolen, then:

```
    If job is valid (non-null func, version matches scheduler):
        Execute job func

    Foreach dependent:
        Decrement dependent dependency count.
        If dependent dependency count is now 0, submit dependent to scheduler.

    If job is contained in a Fence, alert the fence that the job has been run.

    Decrement scheduler overall job count.
        If overall job count is now 0, signal that the scheduler is empty.
```

## Synchronization

Job synchronization is accomplished via dependencies (Job B is dependent on Job A, etc.), fences (block until specific jobs are complete), or overall scheduler wait/sync (at frame end).

When a fence or the scheduler is waiting, it performs work on the waiting thread. This helps increase job throughput while also avoiding deadlocks.

* `WorkFence::wait` - blocks until the individual jobs added to the fence are complete.
* `WorkScheduler::wait` - blocks until _all_ jobs are complete.

## Deque

The underlying `WorkDeque` is an implementation of the Chase-Lev work-stealing deque:

https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf

Jobs are processed in a LIFO manner by their own thread (`pop`), and FIFO by other threads (`steal`).

## Pooling

Jobs are pooled by in thread-local buffers and a global buffer.

The thread-local buffers can hold 1024 jobs each. Once a local buffer is full, additional allocations overflow into the global job buffer. The global buffer uses paged memory and generally does not shrink.

Local pools are more efficient than the global pool as allocation simply increments the buffer offset. The global pool also increments a buffer offset, but one that is stored in an atomic, and if necessary it must allocate another memory page. While fast, it is still slower than a local pool.

When the work scheduler syncs (`WorkScheduler::wait`), it resets all job pools. This is done efficiently by simply resetting the current offets into each buffer. Because no data is actually cleared during a reset, it is imperative that a `JobHandle` is used as opposed to a raw `Job` pointer. A raw pointer can point to out-of-date memory, whereas a handle is trivially validated via `WorkScheduler::valid`.

