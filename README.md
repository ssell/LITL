# LITL Engine

**Current Version:** v0.3.0

This is a learning engine and very (very) early in development. It is public merely to show that it exists, but not for it to see any use (yet).

## Screenshots

![](media/litl_v0p2_sponza.png)

## Requirements

* C++23
* [Vulkan SDK](https://vulkan.lunarg.com/) with Slang
* [clang-cl](https://github.com/llvm/llvm-project/releases/tag/llvmorg-21.1.8) (select _Add LLVM to PATH_)

There are additional third-party dependencies included as submodules.

    git submodule update --init --recursive

## Features

As the engine is new and still under active development this may be small, but is actively growing.

* **Assets**: asynchronous loading of textures, materials, meshes, and models. [(read more)](docs/assets.md)
* **ECS**: chunk-based Archetypal ECS implementation, compile-time system dependencies for seamless component injection, etc. [(read more)](docs/ecs.md)
* **Input**: polling and callbacks for both raw input and action maps. [(read more)](docs/input.md)
* **Jobs**: work stealing with per-thread and global job pools, priority lanes, dependencies, local and shared data, fences, etc. [(read more)](docs/jobs.md)
* **Logging**: thread-safe with support for multiple sinks (console, file, custom)
* **Renderer**: abstract renderer in `litl-renderer` with a concerete Vulkan 1.4 backend in `litl-renderer-vulkan`. [(read more)](docs/renderer.md)
* **Scene**: transform hierarchy with topologically-sorted updates and pluggable spatial partitioning for culling and queries, driven by ECS structural changes. [(read more)](docs/scene.md)

## Releases

### Version 0.6

_Future release. Planned features include: editor._

### Version 0.5

_Future release. Planned features include: deferred rendering, post-processing stack._

### Version 0.4

_Future release. Planned features include: render targets, render passes, directional light, primary directional light cascaded shadowmaps._

### [Version 0.3 on 2026-10-23](https://github.com/ssell/LITL/releases/tag/v0.3.0)

**Primary focus: Input**

Features include: introduction of `litl-platform`, migration of window logic to `litl-platform`, raw input polling and callbacks, input action map polling and callbacks, updated samples. See the linked tagged release for a more complete list of updates.

### [Version 0.2 on 2026-10-13](https://github.com/ssell/LITL/releases/tag/v0.2.0)

**Primary focus: Assets**

Features include: Task async via coroutines, `litl-import` library, assets subsystem, ECS enhancements (optional components, exclusive execution), engine-level materials and textures, renderer depth-stencil and global texture table, etc. See the linked tagged release for a more complete list of updates.

**Samples:**

* [Sponza](samples/sponza)

### [Version 0.1 on 2026-07-30](https://github.com/ssell/LITL/releases/tag/v0.1.0)

The first version of the engine that has many of the core systems operational: ECS, rendering, jobs, scene, dependency injection, etc. See the linked tagged release for a more complete list of updates.

**Samples:**

* [Renderer](samples/renderer)
* [Triangle](samples/triangle)
* [Boids](samples/boids)
