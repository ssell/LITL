# Third-Party Dependencies

## abseil-cpp

Used for `flat_hash_map` which is employed by caches, etc. as a fast map.

* License: Apache-2.0
* Source: https://github.com/abseil/abseil-cpp
* Included via: CMake `FetchContent`

## Catch2

Used for Unit testing.

* License: BSL-1.0
* Source: https://github.com/catchorg/Catch2
* Included via: CMake `FetchContent`

## glaze

Used for various text file support including: `.json`, `.toml`, and `.beve` (binary JSON).

* License: MIT
* Source: https://github.com/stephenberry/glaze
* Included via: CMake `FetchContent`

## GLFW

Used for cross-platform window creation and window event processing.

* License: Zlib
* Source: https://github.com/glfw/glfw
* Include via: CMake `FetchContent`

## GLM

Used for general math structures and operations.

* License: MIT / Happy Bunny
* Source: https://github.com/g-truc/glm
* Included via: CMake `FetchContent`

## rapidobj

Used for importing of `.obj` mesh files.

* License: MIT
* Source: https://github.com/guybrush77/rapidobj
* Included via: Submodule

## SPIRV-Reflect

Used for SPIRV bytecode reflection.

* License: Apache-2.0
* Source: https://github.com/KhronosGroup/SPIRV-Reflect
* Include via: Submodule

## ufbx

Used for import of `.fbx` mesh files.

* License: MIT / Public Domain
* Source: https://github.com/ufbx/ufbx
* Include via: Manual (v0.23.0)
    * The two files used (individual `.h` and `.c`) were manually copied because ufbx is not configured to be used with `FetchContent` and the repository contains many test model files that pollute submodule usage.

## xxHash

Used for generating hash values for various data caches.

* License: BSD 2-Clause License
* Source: https://github.com/Cyan4973/xxHash
* Include via: Submodule