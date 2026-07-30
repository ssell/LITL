# LITL Engine

This is a learning engine and very (very) early in development. It is public merely to show that it exists, but not for it to see any use (yet).

**Current Version:** v0.1.0

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

## Releases

### [Version 0.1.0 on 2026-07-30](https://github.com/ssell/LITL/releases/tag/v0.1.0)

The first version of the engine that has many of the core systems operational: ECS, rendering, jobs, scene, dependency injection, etc.

**Samples:**

* [Renderer](samples/renderer)
* [Triangle](samples/triangle)
* [Boids](samples/boids)
