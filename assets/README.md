# LITL Assets

The LITL engine looks for an `assets/` directory alongside the executable. Within this assets directory all engine-provided models, shaders, materials, etc. are provided under the `litl` folder.

As such, loading an engine-provided sphere using the engine-provided lit material would be:

```
createModelInstance("litl/models/sphere", "litl/materials/lit");
```

With the sample applications a mix of general LITL assets and sample-specific assets are typically uses. This is achieved using CMake to:

1. Copy this `assets` folder next to the executable.
2. Copy the sample-specific `assets` folder next to the executable.

Resulting in an `assets` folder with both a `litl` subdirectory and the sponza-specific `models` folder.

## Shader Modules

The `assets/litl/shaders` directory is added to the Slang search path to capture any engine-provided modules.

For example, `assets/litl/shaders/litl/core.slang` is available as `import litl.core`.

## Future Use

In the future, the `assets` folder will be project-specific with all engine-specific assets provided via a bundle.