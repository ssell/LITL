# LITL Engine

Brushing the C++ dust off with a new little engine.

## Requirements

* Install the [Vulkan SDK](https://vulkan.lunarg.com/) with Slang

## Engine Compilation

Build from within Visual Studio 2022, or via command line.

From the command line:

```
    mkdir out/build/x64-Debug
    cmake -S . -B out/build/x64-Debug -G "Visual Studio 17 2022"
    cd out/build/x64-Debug
    cmake --build . --config Debug
```

## Shader Compilation

Run the `compile-all.ps1` script to compile all shaders under `assets/shaders` into the `assets/shaders/spirv` directory.

    ./compile-all

Individual shaders can be compiled via Slang command line such as:

    ./slangc.exe hello-world.slang -target spirv -o hello-world.spv
