# LITL Engine

This is a learning engine and very (very) early in development. It is public merely to show that it exists, but not for it to see any use (yet).

**Current Version:** v0.0 

## Requirements

* C++20
* [Vulkan SDK](https://vulkan.lunarg.com/) with Slang
* [clang-cl](https://github.com/llvm/llvm-project/releases/tag/llvmorg-21.1.8) (select _Add LLVM to PATH_)

There are additional third-party dependencies included as submodules.

    git submodule update --init --recursive

## Features

As the engine is new and still under active development this may be small, but is actively growing.

* **Renderer**: abstract renderer in `litl-renderer` with a concerete Vulkan 1.4 backend in `litl-renderer-vulkan`. [(read more)](docs/renderer.md)
* **ECS**: chunk-based Archetypal ECS implementation, compile-time system dependencies for seamless component injection, etc. [(read more)](docs/ecs.md)
* **Jobs**: work stealing with per-thread and global job pools, priority lanes, dependencies, local and shared data, fences, etc. [(read more)](docs/jobs.md)
* **Scene**: transform hierarchy with topologically-sorted updates and pluggable spatial partitioning for culling and queries, driven by ECS structural changes. [(read more)](docs/scene.md)
* **Logging**: thread-safe with support for multiple sinks (console, file, custom)

## Samples

Samples are provided as a demonstration and live documentation of LITL. New samples will be added for each release to highlight new features. Below is a list of samples, ordered by their associated release version.

**v0.1** *

* [Renderer](samples/renderer)
* [Triangle](samples/triangle)

_* denotes a future release. The associated samples are not guaranteed to yet be done or stable._