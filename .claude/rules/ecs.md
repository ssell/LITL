---
paths:
  - "litl/ecs/**/*.{cpp,hpp}"
  - "tests/src/litl-ecs/**/*.{cpp}"
---

# litl-ecs Rules

- Custom archetypal ECS implementation.
- Components must be trivially relocatable. No std::list, std::map, or types holding self-referential pointers.
- Archetype graph edges are cached on first transition; never invalidate edges except on archetype destruction.
- Queries are compile-time built from template parameter packs. Don't introduce runtime query construction without discussion.
- System scheduling is multi-threaded based on custom Jobs implementation.
- Systems are run in parallel based on a generated DAG that allows systems that do not share direct or indirect (component-write) dependencies to run at the same time.
- Structural ECS commands flow into per-thread Command Buffers (via EntityCommands).
- Sync points between parallel system layers combine, deduplicate, and process the structure command buffers. Callbacks after the ECS sync point allow for external operations, such as scene graph updates, to occur.