# Work System

Comprised of the following:

* `Job`
* `Worker`
* `WorkScheduler`
* `WorkDeque`

This is the main async execution scheduler for the LITL Engine.

Arbitrary execution units, in the form of function pointers, can be submitted to the `WorkScheduler` in the form of a `Job`. These jobs are then placed into a thread-specific deque and will be executed as soon as possible. Jobs are designed to be fire-and-forget and do not return any results.

## Details

Working from high-level to low, we start with the `Worker` and `Job`.

A `Job` is a function pointer (`JobFunc`) combined with an optional `userData` buffer. It can be dependent on other jobs and likewise can have jobs be dependent on it.