# LITL Libraries

Each subdirectory contains an individual static library that implements a focused set of features for the LITL Engine.

The end user typically does not need to link any of these libraries, aside from `litl-engine` which pulls them in as-needed based on the build configuration (which renderer, which platform, etc.).

```
                               litl-engine
                                    │
     ┌──────────────┬───────────────┼─────────────────┬─────────────────────┐
     │              │               │                 ┆ (opt)               ┆ (opt)
     │              ▼               ▼                 ▼                     ▼
     │          litl-ecs       litl-import   litl-platform-glfw   litl-renderer-vulkan
     │              │               │                 │                     │
     │              │               │                 ▼                     ▼
     │              │               │           litl-platform         litl-renderer
     │              │               │                 │                     │
     ▼              ▼               ▼                 ▼                     ▼
 ┌────────────────────────────────────────────────────────────────────────────────────┐
 │                                     litl-core                                      │
 └────────────────────────────────────────────────────────────────────────────────────┘
```