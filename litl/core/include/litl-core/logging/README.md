# LITL Logger

The LITL Logger is an asynchronous logger implementation which supports logging from any thread to one or more sinks.

By default the logger provides two sinks: console and file. Custom sinks may be added via `LITL::Core::Logger::addSink`.

## Usage

Prior to first use, the Logger needs to be initialized. This is done automatically when creating `Engine`.

    LITL::Core::Logger::initialize(...)

Once initialized, messages can be logged as any of the following severity levels. In order of ascending priority:

* Trace
* Debug
* Info
* Warning
* Error
* Critical

Messages may be filtered out based on severity via the `LITL_LOG_LEVEL` macro.

    #include "litl-core/logging/logLevel.hpp"
    #define LITL_LOG_LEVEL LITL_LOG_LEVEL_INFO
    #include "litl-core/logging/logging.hpp"

This will prevent any messages below Info severity (Trace and Debug) from being processed.

Messages are logged either directly or via convenience forwarding functions:

    LITL::Core::Logger::error("Response returned error code: ", errorCode, ". Message: ", errorMessage);

    // or 

    LITL::logError("Response returned error code: ", errorCode, ". Message: ", errorMessage);

Additionally a message may be logged along with capturing it's source file and line number. While this may be useful for error/critical error messages it is generally not recommended due to the inherit downsides associated with macro-based utilities.

    LITL_LOG_CRITICAL_CAPTURE("Something really bad happened!");
    --> "Something really bad happened! [/path/to/source.cpp@55]"

## Design

While message formatting occurs on the calling thread, the rest of the process is handled on dedicated worker threads to avoid any blocking to other threads that may be caused by an influx of messages and/or slowdowns in the various sinks.

The main `LogProcessor` runs on it's own thread and is fed messages via a Multiple-Producers Single-Consumer (MPSC) concurrent queue (`LITL::Core::Containers::ConcurrentQueue`). When the processor dequeues a message it then hands it off to each instance of `LoggingSink`.

Each sink, like the primary processor, run on their own threads and are fed by dedicated queues. However the sinks use a Single-Producer Single-Consumer (SPSC) concurrent queue (`LITL::Core::Containers::ConcurrentSingleQueue`) which is implemented via a lock-less ring buffer. As the sinks operate on independent threads, any slowness they may encounter does not affect any other processes - logging or otherwise.

                                                                                 ┌ Thread B
                                                               ┌────────────┐    ┌──────────────────┐
    ┌──────────┐                                           ┌──>│ SPSC Queue │───>│ Sink 0 (Console) │
    │ Thread 0 │──┐                                        │   └────────────┘    └──────────────────┘
    └──────────┘  │                      ┌ Thread A        │                     ┌ Thread C
    ┌──────────┐  │   ┌────────────┐    ┌┴──────────────┐  │   ┌────────────┐    ┌──────────────────┐
    │ Thread 1 │──┼──>│ MPSC Queue │───>│ Log Processor |──┼──>│ SPSC Queue │───>│ Sink 1 (File)    │
    └──────────┘  │   └────────────┘    └───────────────┘  │   └────────────┘    └──────────────────┘
    ┌──────────┐  │                                        │                     ┌ Thread D
    │ Thread 2 │──┘                                        │   ┌────────────┐    ┌──────────────────┐
    └──────────┘                                           └──>│ SPSC Queue │───>│ Sink N           │
                                                               └────────────┘    └──────────────────┘