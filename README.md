# LITL Engine

Brushing the C++ dust off with a new little engine.

This is a learning engine and very (very) early in development. It is public merely to show that it exists, but not for it to see any use (yet).

## Requirements

* [Vulkan SDK](https://vulkan.lunarg.com/) with Slang
* [clang-cl](https://github.com/llvm/llvm-project/releases/tag/llvmorg-21.1.8) (select _Add LLVM to PATH_)

## Features

As the engine is new and still under active development this may be small, but is actively growing.

* Logging: thread-safe with support for multiple sinks (console, file, custom)
* ECS: chunk-based Archetypal ECS implementation, compile-time system dependencies for seamless component injection, etc.
* Jobs: work stealing with per-thread and global job pools, priority lanes, dependencies, local and shared data, fences, etc.

More in-depth documentation is (or will) be found in the appropriate subfolders.