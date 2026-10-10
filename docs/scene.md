# LITL Scene — Design Reference

A high-level overview of the scene system in `litl-engine` (`litl/engine/include/litl-engine/scene`): hierarchy, world transforms, spatial partitioning, cameras, and the bridge from ECS structural changes to scene state.

## Overview

The scene layer answers two questions the ECS deliberately doesn't:

- **Who is whose parent?** — the transform hierarchy (parent → child), and the topologically-sorted order in which world matrices must be computed.
- **What is near here?** — spatial queries (what intersects this AABB / sphere / frustum), for culling and broad-phase work.

It sits *on top of* `litl-ecs` and is driven by it. The ECS emits a stream of `EntityChange` records at each sync point; the scene consumes that stream and keeps its hierarchy and spatial index in step with the entities' archetypes. This is the other half of the note in [`ecs.md`](./ecs.md): *"Hierarchy resolution lives outside the library — `litl-ecs` emits `SetParent` changes; applying them is a scene-layer concern."* That application happens here.

The pieces:

- **`SceneManager`** — the public entry point. Owns one or more `Scene`s, tracks the active one, and routes ECS changes into it.
- **`Scene`** — a single world's tracked entities: a `SceneGraph`, a `ScenePartition`, a `SceneTransforms` buffer, and `SceneCameras`. Not thread-safe; mutated only at sync points and in its once-per-frame `onPreRender`.
- **`SceneGraph`** — the parent/child hierarchy in flattened parallel arrays, plus the per-entity GPU-buffer (world-matrix) index and a topological sort.
- **`SceneTransforms`** — the per-frame world matrices, indexed by each entity's GPU-buffer index.
- **`SceneCameras`** — the cameras in the `ObjectPool`, sorted by process order, plus the designated main camera.
- **`ScenePartition`** — a compile-time concept for spatial acceleration; `UniformGridPartition` is the real implementation, `NullPartition` the no-op.
- **`SceneView`** — a parallel-safe handle to the active scene, for use inside systems.
- **`SceneChangeProcessor`** — translates ECS `EntityChange`s into scene `track`/`untrack`/`setParent` calls and cascades destroys to descendants.

The governing idea mirrors the ECS: **structural scene changes are deferred and applied single-threaded at sync points; derived state (world matrices, world bounds, partition placement, cameras) is recomputed once per frame just before `PreRender`; reads are available to parallel systems through a view.**

---

## How it connects to the ECS

The wiring lives in `EngineCallbacks::setup`, which installs two ECS `FrameCallbacks` hooks. The first is `onSyncPoint`:

```cpp
m_impl->engineFrameCallbacks->onSyncPoint = [this](ServiceProvider& services, SystemGroup group, std::span<EntityChange const> entityChanges)
{
    m_impl->sceneManager->processEntityChanges({}, *m_impl->world, entityChanges);
    m_impl->userFrameCallbacks->invokeSyncPoint(services, group, entityChanges);
};
```

(The `{}` is an `Authority<EngineCallbacks>` passkey — only `EngineCallbacks` can call `processEntityChanges`.)

The second is the `PreRender` group's `onPreGroup`, which calls `SceneManager::onPreRender` → `Scene::onPreRender` once per frame before any `PreRender` system runs. See [Per-frame update](#per-frame-update--onprerender).

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
   SceneChangeProcessor::process → Scene mutations (graph marked dirty)
        │
        ⋮   (more layers / groups)
        │
   onPreGroup(PreRender) → SceneManager::onPreRender → Scene::onPreRender
        │
        ▼
   graph re-sort, world matrices, world bounds, partition update, cameras
```

The scene never observes individual `EntityCommands`. It only sees the *outcome* — the post-move `EntityChange` records with their `prevArchetype` / `currArchetype` — and decides what that means for the hierarchy and partition.

---

## SceneChangeProcessor

`process(scene, world, entityChanges)` is where ECS semantics become scene semantics. It sorts a local copy of the changes and then dispatches each. It does not re-sort the graph itself — structural changes only mark the graph dirty, and the re-sort happens in `Scene::onPreRender`.

### Sort order

Commands are processed in the following order:

(`Create Entity` → `Change Archetype` → `SetParent`) → `DestroyEntity`

### The four change types

| `EntityChangeType` | Scene action |
|--------------------|--------------|
| `CreateEntity` | Nothing. A freshly created entity has no components, so there is nothing to track yet. |
| `DestroyEntity` | If the entity is present in the scene, gather it and all of its descendants; `untrack` each and `destroyImmediate` it in the ECS. See below. |
| `ChangeArchetype` | Compare `prev`/`curr` archetypes for `Transform` and `WorldBounds`; `track`/`untrack`/`update` accordingly. |
| `SetParent` | `scene.setParent(entity, parent)`. |

#### Archetype Change

The pivot component is **`Transform`**. 

An entity is in the scene if and only if it has a `Transform`; gaining one (`!prevHadTransform && currHasTransform`) tracks it, losing one untracks it. `WorldBounds` is the secondary signal: losing it falls back to a unit-cube AABB around the transform's position. Otherwise world bounds are derived each frame in `Scene::onPreRender` from the entity's `LocalBounds` (see below).

This leans on the ECS processor's guarantees: a destroy already cancels that entity's other commands, and adds/removes are already collapsed into a single archetype move. The scene processor therefore never has to untangle conflicting changes — it reacts to a clean, final per-entity delta. The archetype ids are resolved back to `Archetype*` via `ArchetypeRegistry::getById`, and `hasComponent<Transform>()` / `hasComponent<WorldBounds>()` answer the gain/loss questions.

#### Entity Destruction

All destroys happen together, after every other command has processed. This is opposite to how Entity commands are processed which handle destroys first to eliminate unnecessary commands.

When destroys are processed, every Entity has already changed archetypes (potentially losing `Transform` and untracking) and switched parents and so the scene is in a stable state. As destroys cascade all affected entities are gathered - both the explicitly destroyed entities and all of their children. These are then removed from the scene.

The gathered set is deduplicated (a child may have been both explicitly destroyed and gathered transitively), and every entity in it is `untrack`ed and then destroyed with `World::destroyImmediate`. The immediate call is safe here because the processor only runs at a sync point. So destroying a parent via `EntityCommands` destroys its whole subtree in both the scene and the ECS.

---

## Scene — graph, partition, transforms, cameras

`Scene` is constructed from a `SceneConfiguration` (partition type + `UniformGridOptions`) along with the `Renderer`, `ObjectPool`, and `World`, and holds its members directly:

```cpp
SceneTransforms      m_transforms;   // world matrices, indexed by GPU-buffer index
SceneGraph           m_graph;
ScenePartitionVariant m_partition;   // std::variant<NullPartition, UniformGridPartition>
SceneCameras         m_cameras;
```

Structural operations fan out to the graph and the partition: `track` adds to both; `untrack` removes from both; `update` adjusts only the partition (bounds moved, hierarchy unchanged). The partition is a `std::variant`, so the concrete strategy is chosen at scene construction from `SceneConfiguration::partition` and dispatched with `std::visit` — no virtual calls, no heap indirection for the partition interface.

```cpp
void Scene::track(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept
{
    m_graph.add(entity, transform);
    std::visit([&](auto& partition) { partition.add(entity, bounds); }, m_partition);
}
```

`track` with no explicit bounds uses a unit cube (`fromCenterHalfExtents(position, {0.5, 0.5, 0.5})`).

`Scene` is explicitly **not thread-safe** and is documented as something you should not touch directly. Structural changes go through `EntityCommands` (and arrive via the processor); reads go through `SceneView`.

### Per-frame update — `onPreRender`

`Scene::onPreRender` runs once per frame, from the `PreRender` group's `onPreGroup` hook (before any `PreRender` system), and is the only place derived scene state is recomputed:

1. **`m_graph.update()`** — re-sorts the hierarchy if anything dirtied it, (re)assigning GPU-buffer indices.
2. **`m_transforms.reserve(m_graph.count())`**, then **`partition.preUpdate()`**.
3. **World matrices** — walk `m_sortedNodes` front to back. For each entity: read its local `Transform`, multiply by the parent's already-computed world matrix if it has a parent, and store the result in `SceneTransforms` at the entity's GPU index. The parent-before-child sort order is what makes this a single pass.
4. **World bounds** — if the entity has a `LocalBounds`, transform it by the world matrix, `update` the partition with the result, and `setComponent<WorldBounds>` (a no-op if the entity has no `WorldBounds`), stamping it with the transform's version.
5. **Cameras** — refresh `SceneCameras` from the `ObjectPool`, set the main camera's aspect ratio from the swapchain, and update each camera with its entity's world matrix.

Because this runs once per frame, `getWorldMatrix` / `getWorldPosition` reads made during groups that run before `PreRender` see the previous frame's values.

### Cameras

`SceneCameras` holds up to `MaxSceneCameras` (32) cameras, re-read from the `ObjectPool` and sorted by process order on each update. One camera can be designated the **main camera** (`setMainCamera(CameraHandle)`), which is the one rendered to the primary swapchain target.

---

## SceneGraph — the hierarchy

The graph has three jobs: an iterable parent → child structure, random access for `entity → parent` and `entity → children`, and ownership of each entity's **index into the world-matrix GPU buffer**. `Scene` is a `friend` and reads the arrays directly during `onPreRender`.

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

`update()` is a no-op unless something dirtied the graph. When dirty, it rebuilds `m_sortedNodes` via an iterative **DFS pre-order** starting from every root (a node with a null parent), assigning depth and GPU index as it goes:

```cpp
for each occupied root:  depth = 0;  push
while frontier:
    node = pop
    gpuIndex[node] = sortedNodes.size()
    sortedNodes.push_back(node)
    for each child: depth[child] = depth[node] + 1; push
```

The invariant this buys: **a parent always precedes its children in `m_sortedNodes`.** That is exactly the order a world-matrix pass needs — compute a parent's world matrix, then multiply each child's local transform by it — so `Scene::onPreRender` walks `m_sortedNodes` once, front to back, with every parent already resolved. A final assert checks the sorted size equals the live node count.

**GPU index = position in the sorted order.** This keeps `SceneTransforms` dense and hierarchy-ordered, but it also means **GPU indices are reassigned whenever the graph is re-sorted** (any create, destroy, or reparent). Don't cache a GPU index across frames; look it up when needed. A newly `add`ed node has a null GPU index until the next `update()`.

### Structural operations

- **`add`** — asserts the slot is vacant; if the transform names a parent, wires it into `m_childNodes`; marks dirty.
- **`setParent`** — unlinks from the previous parent's child list, links into the new one (or leaves it a root if parent is null); marks dirty. Asserts against self-parenting and against parenting to an absent entity.
- **`remove`** — unlinks from its parent, detaches its children (their parent becomes null, so they become roots), and vacates its own slot; marks dirty.

A note on `remove`: it intentionally does not cascade to its children. Removing a parent through `SceneGraph` alone promotes its children to roots rather than removing them. This keeps `remove` simple and defines a clear separation of concerns: if children are to be removed, then those must also be individually removed.

While this appears restrictive on the surface, it has little bearing for most use-cases. That is because it is rare/discouraged for a user to directly manipulate the scene. Instead the scene is populated/transformed automatically by the `SceneChangeProcessor` which itself reacts to changes made via `EntityCommands`. So even though `SceneGraph::remove` does not cascade, destroying an Entity via `EntityCommands` does cascade and all of its descendants are likewise destroyed.

---

## Spatial partition

### The concept

`ScenePartition` is a C++20 concept, not a base class — partitions are duck-typed at compile time and stored by value in the scene's variant:

```cpp
template<typename T>
concept ScenePartition = requires(T p, T const cp, Entity e,
    bounds::AABB const& aabb, bounds::Sphere const& s, bounds::Frustum const& f,
    World& world, ComponentTypeId componentType, uint32_t limit,
    std::vector<PartitionQueryResult>& out)
{
    { p.add(e, aabb) }    noexcept -> std::same_as<void>;
    { p.remove(e) }       noexcept -> std::same_as<void>;
    { p.preUpdate() }     noexcept -> std::same_as<void>;
    { p.update(e, aabb) } noexcept -> std::same_as<void>;
    { cp.query(aabb, out, limit) }                      noexcept -> std::same_as<void>;
    { cp.query(aabb, world, componentType, out, limit) } noexcept -> std::same_as<void>;
    // + the same two overloads for Sphere and Frustum
};
```

Queries return `PartitionQueryResult { entity, worldPosition, distanceSquared }`. Each query has a component-filtered overload that only returns entities that have the given component, and a `limit` (0 = unlimited) that stops gathering once reached. `Scene`/`SceneView` add a `sorted` flag that sorts results by `distanceSquared` from the query's center.

All positions, bounds, and queries are **world-space**. `NullPartition` satisfies the concept with empty bodies — use it when a scene needs hierarchy but no spatial queries. Both implementations carry a `static_assert(ScenePartition<...>)` so a contract drift is a compile error at the definition site.

### UniformGridPartition

The shipped strategy is a **2D uniform grid on the XZ plane** (Y is ignored for bucketing; each cell's AABB simply spans `[yMin, yMax]`). Configuration comes from `UniformGridOptions`:

- `origin`, `cellSize`, `cellCount` — both `cellSize` and `cellCount` must be powers of two greater than 1 (validated by `isValid()`); `fromWorldSize(...)` is a convenience that rounds to fit a desired world extent.
- `yMin` / `yMax` — vertical span of every cell's AABB.

An entity maps to a cell by `(getCellIndexX(x), getCellIndexZ(z))`, flattened as `cellX + cellZ * cellCount`. Two wrinkles:

- **Oversized entities.** If an entity's XZ half-extents exceed a threshold (`cellSize² · 0.5`), it would smear across too many cells, so it goes into a single dedicated **overflow cell** instead and is always considered by every query. `getOversizedCellPopulation()` reports how many.
- **Range queries.** An AABB / sphere / frustum query computes the cell range from the query's XZ bounds (`getCellIndexX(min.x)` … `getCellIndexX(max.x)`, same for Z), gathers candidates from every covered cell plus the overflow cell, and appends them to the caller's vector. The grid is a broad phase — it returns *candidates that share cells*, not an exact intersection set; precise tests are the caller's job.

`update(entity, bounds)` re-buckets an entity only if it has moved enough to change cells, keeping churn cheap for mostly-stationary objects.

`add` does not place the entity in a cell yet. At `add` time there is no reliable world-space position (world matrices haven't been computed), so the entity is parked in a "new entities" set and placed into its cell on its **first** `update`, which comes from `Scene::onPreRender`. Until then it won't appear in query results. Entities without a `LocalBounds` never receive that update, so they stay out of the grid.

---

## SceneView — parallel-safe reads

Systems run in parallel across chunks and must not touch the mutable `Scene`. `SceneView` is the system-facing face of the active scene. It holds a `std::shared_ptr<Scene>` and exposes:

- **Hierarchy:** `isPresent`, `getParent`, `getChildren`, `getGpuBufferIndex`.
- **Transforms:** `getWorldMatrix`, `getWorldMatrices`, `getWorldPosition` (all as of the last `onPreRender`).
- **Queries:** AABB / Sphere / Frustum, each with `sorted` and `limit` parameters, a `ComponentTypeId`-filtered overload, and a `query<T>(...)` convenience.
- **Cameras:** `getCameras`, `getMainCamera`, `getMainCameraHandle`, `setMainCamera`.

Two members are **not** parallel-safe: `track(...)` (both overloads), which is documented as main-thread / sync-point only and unnecessary for entities created through `EntityCommands`, and `setMainCamera`, which writes scene state.

It's registered as a service (`SceneView` singleton) and handed the active scene by `SceneManager::setActiveScene` via the `setViewedScene` friend hook. A system that needs "what's in this frustum?" or "what's my parent's world matrix?" pulls the `SceneView` from the service provider and queries it; anything structural goes back through `EntityCommands` and lands at the next sync point. The engine's `CullingSystem` (`PreRender`) is the canonical consumer: its `prepare()` frustum-queries the view once per camera, and its `update` filters renderable entities against those results.

The safety argument: during system execution the scene is **not** being mutated. Mutations only happen in `processEntityChanges` (between layers) and `onPreRender` (before the `PreRender` group starts), both on the calling thread while no systems are running, so concurrent reads through the view are race-free by construction.

---

## SceneManager — ownership and routing

`SceneManager` is the public surface and the engine's integration point. It owns the scenes and the single `SceneChangeProcessor`:

```cpp
struct SceneManager::Impl
{
    std::vector<std::shared_ptr<Scene>> scenes;
    std::shared_ptr<SceneView>          view;
    std::shared_ptr<ObjectPool>         objectPool;
    std::shared_ptr<RenderManager>      renderManager;
    std::shared_ptr<World>              world;
    SceneChangeProcessor                sceneChangeProcessor;
    uint32_t                            activeIndex{ Constants::uint32_null_index };
};
```

`createScene(config)` appends a scene (constructed with the renderer, object pool, and world) and, if it is the first scene, makes it active automatically. `setActiveScene(index)` swaps the active index and re-points the shared `SceneView`. `processEntityChanges` and `onPreRender` forward to the active scene (and no-op if there are no scenes); both take an `Authority<EngineCallbacks>` passkey, and `setup` takes an `Authority<Engine>`. `setup` injects the `SceneView`, `ObjectPool`, `RenderManager`, and `World` services and fatally asserts if any are missing.

Multi-scene support is mostly scaffolding today — `setActiveScene` carries a `todo` for whatever a full scene swap entails (re-extracting GPU state, etc.).

---

## Conventions and invariants

### Transform is the membership key

An entity participates in the scene exactly when it has a `Transform`. `Transform` stores the *local* transform (position, rotation, uniform scale) and a `ParentEntity`; if the parent is null, local == world. It also carries a `version` stamped with `World::getVersion()` on every mutation, which is copied into `WorldBounds::version` when the bounds are recomputed. `Transform::create(mat4)` decomposes a matrix into a `Transform`, but only uniform scale is representable — non-uniform scale is currently dropped. The parent field is guarded by the `ParentEntityWriteKey` passkey (see [`ecs.md`](./ecs.md)) so it can't be reassigned directly inside a system — reparenting must go through a deferred `setParent` command.

### Deferred structure, immediate reads

Like the ECS, all structural scene changes are applied single-threaded at sync points; reads happen any time through `SceneView`. Never mutate a `Scene` from inside a parallel system.

### Derived state is once per frame

World matrices, `WorldBounds`, partition placement, and camera state are recomputed only in `Scene::onPreRender`. Anything reading them earlier in the frame sees the previous frame's values, and newly created entities have no world matrix or partition cell until their first `onPreRender`.

### Sorted order is a contract

`m_sortedNodes` guarantees parent-before-child. Downstream world-matrix propagation depends on it; if you add a traversal that assumes a different order, re-derive it rather than reusing this array.

### Power-of-two grid

`UniformGridOptions` must validate (`isValid()`) before use — non-power-of-two cell size or count is rejected. The grid buckets on XZ only.

---

## What's not yet here

Gaps worth knowing about, for context on the current shape:

- **World-matrix pass is single-threaded.** `Scene::onPreRender` walks every tracked entity on one thread and reads each `Transform` / `LocalBounds` through `World::getComponent` (a copy per call), even for entities whose transform hasn't changed.
- **GPU indices aren't stable.** They're reassigned on every graph re-sort, so per-entity GPU data must be re-uploaded or re-indexed after structural changes.
- **Non-uniform scale.** `Transform` holds a uniform scale only; `Transform::create(mat4)` drops non-uniform scale (a `todo` notes a future `NonUniformScale` component).
- **`SceneCulling` is a placeholder.** Culling currently lives in the ECS `CullingSystem`; the `SceneCulling` class is empty.
- **One partition strategy.** Only `UniformGrid` (plus `NullPartition`); no octree / BVH / loose grid, and the grid is 2D (XZ) — tall scenes get no vertical discrimination.
- **Limited cycle protection.** `setParent` asserts against direct self-parenting and absent parents, but there's no deep cycle check (A→B→A).
- **Multi-scene swap.** `setActiveScene` re-points the view but the full swap path is a `todo`.

Each is a deliberate deferral; none is locked out by the current structure.

---

## Useful files to read

When the document is no longer enough, these are the load-bearing files:

| File | What lives here |
|------|-----------------|
| `litl/engine/include/litl-engine/scene/scene.hpp` | `Scene` public surface — track / untrack / query / `onPreRender` / cameras |
| `litl/engine/src/scene/scene.cpp` | Graph + partition fan-out, `std::variant` dispatch, the per-frame world-matrix / bounds / camera pass |
| `litl/engine/include/litl-engine/scene/sceneGraph.hpp` | Parallel-array layout, the flattened-node rationale |
| `litl/engine/src/scene/scenegraph.cpp` | DFS topological sort, GPU-index assignment, parent wiring |
| `litl/engine/include/litl-engine/scene/sceneTransforms.hpp` | World-matrix storage indexed by GPU index |
| `litl/engine/include/litl-engine/scene/sceneCameras.hpp` | Camera list and main-camera tracking |
| `litl/engine/src/scene/sceneChangeProcessor.cpp` | `EntityChange` → scene action translation (the ECS bridge), destroy cascade |
| `litl/engine/include/litl-engine/scene/partition/scenePartition.hpp` | The `ScenePartition` concept |
| `litl/engine/src/scene/partition/uniformGridPartition.cpp` | XZ grid, oversized overflow cell, range queries |
| `litl/engine/include/litl-engine/scene/sceneView.hpp` | Parallel-safe read interface |
| `litl/engine/src/scene/sceneManager.cpp` | Scene ownership, active-scene routing |
| `litl/engine/src/engineCallbacks.cpp` | Where `onSyncPoint` and the `PreRender` pre-group hook are wired to the `SceneManager` |
| `litl/engine/src/ecs/systems/cullingSystem.cpp` | Main consumer of `SceneView` frustum queries |
| `litl/engine/include/litl-engine/ecs/components/transform.hpp` | The `Transform` component and its parent/version fields |

For the upstream half of this pipeline — how `EntityChange`s are produced — see [`ecs.md`](./ecs.md).
