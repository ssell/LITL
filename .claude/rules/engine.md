---
paths:
  - "litl/engine/**/*.{cpp,hpp}"
  - "tests/src/litl-engine/**"
---

# litl-engine Rules

- Frame life cycle controlled via the ECS library (litl-ecs) with callbacks at sync points and other frame points.
- Services (Engine, Renderer, World, etc.) provided via custom dependency injection defined in litl-core