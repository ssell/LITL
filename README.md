# LITL Engine

This is a learning engine and very (very) early in development. It is public merely to show that it exists, but not for it to see any use (yet).

## Requirements

* C++20
* [Vulkan SDK](https://vulkan.lunarg.com/) with Slang
* [clang-cl](https://github.com/llvm/llvm-project/releases/tag/llvmorg-21.1.8) (select _Add LLVM to PATH_)

There are additional third-party dependencies included as submodules.

    git submodule update --init --recursive

## Features

As the engine is new and still under active development this may be small, but is actively growing.

* **Renderer**: abstract renderer in `litl-renderer` with a concerete Vulkan 1.4 backend in `litl-renderer-vulkan`. [(read more)](docs/renderer.md)
* **ECS**: chunk-based Archetypal ECS implementation, compile-time system dependencies for seamless component injection, etc.
* **Jobs**: work stealing with per-thread and global job pools, priority lanes, dependencies, local and shared data, fences, etc. [(read more)](docs/jobs.md)
* **Logging**: thread-safe with support for multiple sinks (console, file, custom)