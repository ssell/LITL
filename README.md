# Learning Vulkan

## Requirements

* Install the [Vulkan SDK](https://vulkan.lunarg.com/) with GLM and Slang

## Shader Compilation

Run the `compile-all.ps1` script to compile all shaders under `assets/shaders` into the `assets/shaders/spirv` directory.

    ./compile-all

Individual shaders can be compiled via Slang command line such as:

    ./slangc.exe hello-world.slang -target spirv -o hello-world.spv
