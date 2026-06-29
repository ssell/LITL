# LITL Scene — Design Reference

A high-level overview of the scene system in `litl-engine` (`litl/engine/include/litl-engine/scene`): hierarchy, spatial partitioning, and the bridge from ECS structural changes to scene state.

## Overview

The scene layer answers two questions the ECS deliberately doesn't:

- **Who is whose parent?** — the transform hierarchy (parent → child), and the topologically-sorted order in which world matrices must be computed.
- **What is near here?** — spatial queries (what intersects this AABB / sphere / frustum), for culling and broad-phase work.

It sits *on top of* `litl-ecs` and is driven by it. The ECS emits a stream of `EntityChange` records at each sync point; the scene consumes that stream and keeps its hierarchy and spatial index in step with the entities' archetypes. This is the other half of the note in [`ecs.md`](./ecs.md): *"Hierarchy resolution lives outside the library — `litl-ecs` emits `SetParent` changes; applying them is a scene-layer concern."* That application happens here.

The pieces:

- **`SceneManager`** — the public entry point. Owns one or more `Scene`s, tracks the active one, and routes ECS changes into it.
- **`Scene`** — a single world's tracked entities: a `SceneGraph` plus a `ScenePartition`. Not thread-safe; mutated only at sync points.
- **`SceneGraph`** — the parent/child hierarchy in flattened parallel arrays, plus the per-entity GPU-buffer (world-matrix) index and a topological sort.
- **`ScenePartition`** — a compile-time concept for spatial acceleration; `UniformGridPartition` is the real implementation, `NullPartition` the no-op.
- **`SceneView`** — a read-only, parallel-safe handle to the active scene, for use inside systems.
- **`SceneCommandProcessor`** — translates ECS `EntityChange`s into scene `track`/`untrack`/`setParent` calls.

The governing idea mirrors the ECS: **structural scene changes are deferred and applied single-threaded at sync points; reads are available to parallel systems through a view.**

---

## How it connects to the ECS

The wiring lives in `EngineCallbacks::setup`, which installs the ECS `FrameCallbacks::onSyncPoint` hook:

```cpp
m_pFrameCallbacks->onSyncPoint = [this](SystemGroup group, std::span<EntityChange const> entityChanges)
{
    m_pSceneManager->processEntityChanges(*m_pWorld, entityChanges);
    m_pUserFrameCallbacks->invokeSyncPoint(group, entityChanges);
};
```

Recall from [`ecs.md`](./ecs.md) that `onSyncPoint` fires after each system layer's command buffers are processed, carrying the `EntityChange` list the `EntityCommandProcessor` produced. So the flow end-to-end is:

```
system records EntityCommands  (per-thread, during parallel execution)
        │
   layer completes  → ECS EntityCommandProcessor runs
        │              (combine, sort, one archetype move per entity)
        ▼
   EntityChange[]  (Create / Destroy / ChangeArchetype / SetParent)
        │
   onSyncPoint → SceneManager::processEntityChanges
        │
        ▼
   SceneCommandProcessor::process → Scene mutations → scene.sync()
```

The scene never observes individual `EntityCommands`. It only sees the *outcome* — the post-move `EntityChange` records with their `prevArchetype` / `currArchetype` — and decides what that means for the hierarchy and partition.

---

## SceneCommandProcessor — interpreting changes

`process(scene, world, entityChanges)` is where ECS semantics become scene semantics. It first sorts a local copy of the changes, then dispatches each, then calls `scene.sync()` once at the end.

### Sort order

```cpp
// SetParent last; everything else grouped by entity then change type.
return std::tie(aIsSetParent, a.entity, a.type) < std::tie(bIsSetParent, b.entity, b.type);
```

`SetParent` is deferred to the very end of the batch so that every entity has been tracked (or untracked) before any parent links are wired — you can't parent to an entity the graph hasn't seen yet. (See the assert in `SceneGraph::setParent` that the parent be present.)

### The four change types

| `EntityChangeType` | Scene action |
|--------------------|--------------|
| `CreateEntity` | Nothing. A freshly created entity has no components, so there is nothing to track yet. |
| `DestroyEntity` | If the entity's `prevArchetype` had a `Transform`, `untrack` it. |
| `ChangeArchetype` | Compare `prev`/`curr` archetypes for `Transform` and `WorldBounds`; `track`/`untrack`/`update` accordingly. |
| `SetParent` | `scene.setParent(entity, parent)`. |

The pivot component is **`Transform`**. An entity is in the scene if and only if it has a `Transform`; gaining one (`!prevHadTransform && currHasTransform`) tracks it, losing one untracks it. `WorldBounds` is the secondary signal: losing it falls back to a unit-cube AABB around the transform's position; it's otherwise maintained by a separate world-bounds system.

```cpp
void SceneCommandProcessor::onChangeArchetype(Scene& scene, World& world, EntityChange const& change) const noexcept
{
    if (change.prevArchetype == change.currArchetype) return;
    // ... resolve prev/curr archetypes via ArchetypeRegistry::getById ...
    // lost Transform  → untrack
    // gained Transform → track (bounds left for WorldBoundsSystem)
    // lost Bounds      → update to unit-cube AABB at transform position
}
```

This leans on the ECS processor's guarantees: a destroy already cancels that entity's other commands, and adds/removes are already collapsed into a single archetype move. The scene processor therefore never has to untangle conflicting changes — it reacts to a clean, final per-entity delta. The archetype ids are resolved back to `Archetype*` via `ArchetypeRegistry::getById`, and `hasComponent<Transform>()` / `hasComponent<WorldBounds>()` answer the gain/loss questions.

---

## Scene — graph + partition

`Scene` is a thin orchestrator over two members held in its `Impl`:

```cpp
struct Scene::Impl
{
    SceneGraph graph;
    ScenePartitionVariant partition;   // std::variant<NullPartition, UniformGridPartition>
};
```

Every mutating operation fans out to both: `track` adds to the graph *and* the partition; `untrack` removes from both; `update` adjusts only the partition (bounds moved, hierarchy unchanged). The partition is a `std::variant`, so the concrete strategy is chosen at scene construction from `SceneConfig` and dispatched with `std::visit` — no virtual calls, no heap indirection for the partition interface.

```cpp
void track(Entity e, Transform const& t, bounds::AABB b) noexcept
{
    graph.add(e, t);
    std::visit([&](auto& p){ p.add(e, b); }, partition);
}
```

`track` with no explicit bounds uses a unit cube (`fromCenterHalfExtents(position, {0.5, 0.5, 0.5})`). `Scene::sync()` simply calls `graph.update()` — the partition stays consistent incrementally, but the graph's topological sort is rebuilt lazily (see below).

`Scene` is explicitly **not thread-safe** and is documented as something you should not touch directly. Structural changes go through `EntityCommands` (and arrive via the processor); reads go through `SceneView`.

---

## SceneGraph — the hierarchy

The graph has three jobs: an iterable parent → child structure, random access for `entity → parent` and `entity → children`, and ownership of each entity's **index into the world-matrix GPU buffer**.

### Storage: parallel arrays keyed by entity index

Rather than a node struct with pointers, the graph is a struct-of-arrays, every array indexed directly by `entity.index`:

```cpp
std::vector<Entity>                                  m_nodeToEntity;   // slot → entity (occupancy check)
std::vector<uint32_t>                                m_nodeParent;     // child → parent (null = root)
std::unordered_map<uint32_t, std::vector<uint32_t>>  m_childNodes;     // parent → children
std::vector<uint32_t>                                m_nodeDepth;      // tree depth (0 = root)
std::vector<uint32_t>                                m_nodeGpuIndex;   // → world-matrix buffer slot
std::vector<NodeState>                               m_nodeOccupied;   // Vacant / Present
std::vector<uint32_t>                                m_sortedNodes;    // flattened, topo-sorted
```

`ensureFit(index)` grows all arrays together so they stay the same length — the slot for an entity is just its index, so storage scales with the *highest* entity index seen, not the live count. `isPresent` is the canonical liveness check: not null, in range, the slot's entity matches, and the slot is `Present`.

### The flattened topological sort

`update()` is a no-op unless something dirtied the graph. When dirty, it rebuilds `m_sortedNodes` via an iterative **DFS pre-order** starting from every root (a node with a null parent), assigning depth as it descends:

```cpp
for each occupied root:  depth = 0;  push
while frontier:
    node = pop; sortedNodes.push_back(node)
    for each child: depth[child] = depth[node] + 1; push
```

The invariant this buys: **a parent always precedes its children in `m_sortedNodes`.** That is exactly the order a world-matrix pass needs — compute a parent's world matrix, then multiply each child's local transform by it — so the downstream transform/render system can walk `m_sortedNodes` once, front to back, with every parent already resolved. A final assert checks the sorted size equals the live node count.

### Structural operations

- **`add`** — asserts the slot is vacant; if the transform names a parent, wires it into `m_childNodes`; marks dirty.
- **`setParent`** — unlinks from the previous parent's child list, links into the new one (or leaves it a root if parent is null); marks dirty. Asserts against self-parenting and against parenting to an absent entity.
- **`remove`** — unlinks from its parent, then **cascades**: `removeChildrenOf` walks the whole subtree and vacates every descendant. Destroying a tracked parent therefore drops its entire branch from the graph in one go.

---

## Spatial partition

### The concept

`ScenePartition` is a C++20 concept, not a base class — partitions are duck-typed at compile time and stored by value in the scene's variant:

```cpp
template<typename T>
concept ScenePartition = requires(T p, T const cp, Entity e,
    bounds::AABB const& aabb, bounds::Sphere const& s, bounds::Frustum const& f,
    std::vector<Entity>& out)
{
    { p.add(e, aabb) }    noexcept -> std::same_as<void>;
    { p.remove(e) }       noexcept -> std::same_as<void>;
    { p.update(e, aabb) } noexcept -> std::same_as<void>;
    { cp.query(aabb, out) } noexcept -> std::same_as<void>;   // + Sphere, Frustum overloads (const)
};
```

All positions, bounds, and queries are **world-space**. `NullPartition` satisfies the concept with empty bodies — use it when a scene needs hierarchy but no spatial queries. Both implementations carry a `static_assert(ScenePartition<...>)` so a contract drift is a compile error at the definition site.

### UniformGridPartition

The shipped strategy is a **2D uniform grid on the XZ plane** (Y is ignored for bucketing; each cell's AABB simply spans `[yMin, yMax]`). Configuration comes from `UniformGridOptions`:

- `origin`, `cellSize`, `cellCount` — both `cellSize` and `cellCount` must be powers of two greater than 1 (validated by `isValid()`); `fromWorldSize(...)` is a convenience that rounds to fit a desired world extent.
- `yMin` / `yMax` — vertical span of every cell's AABB.

An entity maps to a cell by `(getCellIndexX(x), getCellIndexZ(z))`, flattened as `cellX + cellZ * cellCount`. Two wrinkles:

- **Oversized entities.** If an entity's XZ half-extents exceed a threshold (`cellSize² · 0.5`), it would smear across too many cells, so it goes into a single dedicated **overflow cell** instead and is always considered by every query. `getOversizedCellPopulation()` reports how many.
- **Range queries.** An AABB / sphere / frustum query computes the cell range from the query's XZ bounds (`getCellIndexX(min.x)` … `getCellIndexX(max.x)`, same for Z), gathers candidates from every covered cell plus the overflow cell, and appends them to the caller's vector. The grid is a broad phase — it returns *candidates that share cells*, not an exact intersection set; precise tests are the caller's job.

`update(entity, bounds)` re-buckets an entity only if it has moved enough to change cells, keeping churn cheap for mostly-stationary objects.

---

## SceneView — parallel-safe reads

Systems run in parallel across chunks and must not touch the mutable `Scene`. `SceneView` is the read-only face of the active scene — it holds a `std::shared_ptr<Scene>` and exposes only const operations: `isPresent`, `getParent`, `getChildren`, `getGpuBufferIndex`, and the three `query` overloads. No `track`, no `setParent`.

It's registered as a service (`SceneView` singleton) and handed the active scene by `SceneManager::setActiveScene` via the `setViewedScene` friend hook. A system that needs "what's in this frustum?" or "what's my parent's GPU index?" pulls the `SceneView` from the service provider and queries it; anything structural goes back through `EntityCommands` and lands at the next sync point.

The safety argument: during system execution the scene is **not** being mutated (mutations only happen in `processEntityChanges`, between layers), so concurrent reads through the view are race-free by construction.

---

## SceneManager — ownership and routing

`SceneManager` is the public surface and the engine's integration point. It owns the scenes and the single `SceneCommandProcessor`:

```cpp
struct SceneManager::Impl
{
    std::vector<std::shared_ptr<Scene>> scenes;
    std::shared_ptr<SceneView>          view;
    SceneCommandProcessor               commandProcessor;
    uint32_t                            activeIndex{ 0 };
};
```

`createScene(config)` appends a scene; `setActiveScene(index)` swaps the active index and re-points the shared `SceneView`. `processEntityChanges(world, changes)` forwards to the active scene's processor (and no-ops if there are no scenes). The `Engine` constructs the manager, calls `setup` to inject the `SceneView` service, and owns the `EngineCallbacks` that route `onSyncPoint` here.

Multi-scene support is mostly scaffolding today — `setActiveScene` carries a `todo` for whatever a full scene swap entails (re-extracting GPU state, etc.).

---

## Conventions and invariants

### Transform is the membership key

An entity participates in the scene exactly when it has a `Transform`. `Transform` stores the *local* transform (position, rotation, uniform scale) and a `ParentEntity`; if the parent is null, local == world. It also carries a `version` stamped with `World::getVersion()` on every mutation, which the world-bounds system uses to know when to recompute. The parent field is guarded by the `ParentEntityWriteKey` passkey (see [`ecs.md`](./ecs.md)) so it can't be reassigned directly inside a system — reparenting must go through a deferred `setParent` command.

### Deferred structure, immediate reads

Like the ECS, all structural scene changes are applied single-threaded at sync points; reads happen any time through `SceneView`. Never mutate a `Scene` from inside a parallel system.

### Sorted order is a contract

`m_sortedNodes` guarantees parent-before-child. Downstream world-matrix propagation depends on it; if you add a traversal that assumes a different order, re-derive it rather than reusing this array.

### Power-of-two grid

`UniformGridOptions` must validate (`isValid()`) before use — non-power-of-two cell size or count is rejected. The grid buckets on XZ only.

---

## What's not yet here

Gaps worth knowing about, for context on the current shape:

- **World-matrix computation isn't in `scene/`.** The graph hands out the sorted order and per-entity GPU index; the actual matrix multiply/propagation lives in a transform system that consumes them. (And GPU-index *assignment* itself is currently stubbed — `add` stores a null index.)
- **Render integration is stubbed.** `EngineCallbacks::onRender` documents the intended path (frustum-cull via the partition → build a draw list of visible `transform-index + mesh + material` → submit to the renderer) but is a `todo`.
- **One partition strategy.** Only `UniformGrid` (plus `NullPartition`); no octree / BVH / loose grid, and the grid is 2D (XZ) — tall scenes get no vertical discrimination.
- **Limited cycle protection.** `setParent` asserts against direct self-parenting and absent parents, but there's no deep cycle check (A→B→A).
- **Subtree destroy semantics.** Removing a tracked parent vacates its descendants' graph nodes but does not destroy those ECS entities; the lifetime coupling is left to the caller.
- **Multi-scene swap.** `setActiveScene` re-points the view but the full swap path is a `todo`.

Each is a deliberate deferral; none is locked out by the current structure.

---

## Useful files to read

When the document is no longer enough, these are the load-bearing files:

| File | What lives here |
|------|-----------------|
| `litl/engine/include/litl-engine/scene/scene.hpp` | `Scene` public surface — track / untrack / query / sync |
| `litl/engine/src/scene/scene.cpp` | Graph + partition fan-out, partition `std::variant` dispatch |
| `litl/engine/include/litl-engine/scene/sceneGraph.hpp` | Parallel-array layout, the flattened-node rationale |
| `litl/engine/src/scene/scenegraph.cpp` | DFS topological sort, cascade removal, parent wiring |
| `litl/engine/src/scene/sceneCommandProcessor.cpp` | `EntityChange` → scene action translation (the ECS bridge) |
| `litl/engine/include/litl-engine/scene/partition/scenePartition.hpp` | The `ScenePartition` concept |
| `litl/engine/src/scene/partition/uniformGridPartition.cpp` | XZ grid, oversized overflow cell, range queries |
| `litl/engine/include/litl-engine/scene/sceneView.hpp` | Parallel-safe read interface |
| `litl/engine/src/scene/sceneManager.cpp` | Scene ownership, active-scene routing |
| `litl/engine/src/engineCallbacks.cpp` | Where `onSyncPoint` is wired to `processEntityChanges` |
| `litl/engine/include/litl-engine/ecs/components/transform.hpp` | The `Transform` component and its parent/version fields |

For the upstream half of this pipeline — how `EntityChange`s are produced — see [`ecs.md`](./ecs.md).
