---
paths:
  - "litl/renderer/**/*.{cpp,hpp}"
  - "litl/renderer-vulkan/**/*.{cpp,hpp}"
---

# litl-renderer-vulkan Rules

- Abstract renderer interface defined in `litl-renderer`
- Vulkan 1.4 backend defined in `litl-renderer-vulkan`
- Communication between abstract layer and backends accomplished using opaque handles and function pointer tables.
- SSBOs to store scene data (transforms, etc.) which is provided via the engine (litl-engine).
- Dynamic rendering and timeline semaphores. No render passes / framebuffers. Descriptor indexing via `VK_EXT_descriptor_indexing` (or core 1.2 features).