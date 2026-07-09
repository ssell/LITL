# LITL ECS — Design Reference

A high-level overview of `litl-ecs`, the engine's archetypal Entity Component System.

## Overview

`litl-ecs` is a single static library (depends only on `litl-core`, exceptions and RTTI off). It implements an **archetypal** ECS: entities with the same component set share storage, and that storage is column-major (struct-of-arrays) inside fixed 16 KB chunks. Iteration is always over the chunks of archetypes that match a query — never over individual entities.

The pieces, from the bottom up:

- **`Entity`** — a 32-bit index + 32-bit version. The version invalidates stale handles after an index is recycled.
- **`Component`** — a plain, standard-layout struct. It carries no base class and no virtuals; the system learns how to construct/move/destroy it through a `ComponentDescriptor` table of function pointers.
- **`Archetype` / `Chunk`** — an archetype is one ordered component set; its entities live in a `PagedVector` of 16 KB `Chunk`s laid out as parallel component columns.
- **`System`** — a user struct with an `update(...)` method. Its parameter list *is* its query: the component types it reads and writes are extracted at compile time.
- **`World`** — the single front door. Owns the system manager, the per-thread command buffers, and the immediate entity operations.
- **`EntityCommands`** — the deferred command buffer. Structural changes requested during system execution are recorded here and applied at sync points.

The central idea everything else serves: **adding or removing a component is just moving an entity from one archetype to another.** There is no per-entity component bag; membership is structural.

---

## Quick start — define, register, run

```cpp
#include "litl-ecs/world.hpp"

using namespace litl;

// 1. Components are plain standard-layout structs.
struct Position { float x, y, z; };
struct Velocity { float dx, dy, dz; };

// 2. A system is a struct with setup(...) and update(...).
//    update() must begin with (EntityCommands&, float); the rest is the query.
struct MovementSystem
{
    // Called one time for the lifetime of the application.
    void setup(ServiceProvider& services) {}

    // Called each frame prior to any calls to update(...)
    void prepare() {}

    // Called each frame according to its SystemGroup and dependencies.
    // There is only a single instance of the System but it is invoked in parallel on separate chunks.
    // The first three parameters (EntityCommands, float, Entity) are required and at least one component.
    void update(EntityCommands& commands, float dt, Entity e, Position& p, Velocity const& v)
    {
        p.x += v.dx * dt;
        p.y += v.dy * dt;
        p.z += v.dz * dt;
    }
};

World world;
world.getSystemCollection().addSystem<MovementSystem>(SystemGroup::Update);

// 3. Spawn some entities (immediate path shown; prefer command buffers in real code).
Entity e = world.createImmediate();
world.addComponentsImmediate(e, Position{}, Velocity{ 1, 0, 0 });

// 4. Per frame.
world.setup(services, callbacks);
world.setupSystems(services);
while (running)
    world.run(dt, fixedStep);
```

`MovementSystem::update` is invoked once per entity that has both `Position` and `Velocity`, with references straight into chunk storage. `Position&` marks a write, `Velocity const&` a read — and that distinction is what the scheduler uses to order systems. See [Systems](#systems).

---

## Architecture

### Entity = index + version

`Entity` is two `uint32_t`s: `index` maps to storage, `version` distinguishes reuses of that index. When an entity is destroyed its index is released for reuse; the next entity to claim it gets `version + 1`. Comparing both fields is required to identify an entity — `index` alone tells you *which slot*, the pair tells you *which occupant*.

Nullness is decided by `index` only (`Entity::null()` carries `null_entity_id`), regardless of version. The 32+32 split is deliberate: it avoids the mask/shift overhead of a packed handle on the indexing that happens millions of times per frame, and a 64-bit entity matches what modern CPUs move cheaply.

### EntityRecord — the entity → storage map

`EntityRecord { Entity entity; Archetype* archetype; uint32_t archetypeIndex; }` is the single source of truth for where an entity's components live. `EntityRegistry` owns these records and is the one place that must stay consistent: it's updated on every create, destroy (which swap-removes), and archetype move. A null `archetype` means the entity is dead.

`EntityRegistry` is a static, single-threaded registry. Direct use is discouraged; go through `World`.

### Archetype — an ordered component set

An `Archetype` is identified by its component set, reduced to a `uint64_t` component hash. The set is **ordered by component id**, so `Archetype<Foo, Bar>` and `Archetype<Bar, Foo>` resolve to the same archetype. `ArchetypeRegistry` owns every archetype, deduplicates by hash, and hands out the empty archetype (`empty_archetype_id == 0`) that every entity starts in.

`ArchetypeRegistry::fetchNewArchetypes()` returns archetypes created since the last call — this is how systems discover storage they should now iterate (see [Archetype matching](#archetype-matching)).

### Chunk — 16 KB of column-major storage

A `Chunk` is a 16 KB byte buffer holding components for up to `max_entities_per_chunk` (512) entities. Its internal layout:

```
[ ChunkHeader ][ entity array ][ column: Component A ][ column: Component B ] ...
```

Components are stored **column-major** (SoA): all `A` values contiguous, then all `B` values. This is what makes a system iterating `A` and `B` cache-friendly — each column is a tight array. The `ChunkHeader` holds the back-pointer to the archetype, the current `count`, the `capacity`, the chunk index, and a `version` stamp (the world version at last modification, for future change detection).

Removal is **swap-and-pop**: `removeAndSwap` moves the last entity into the vacated slot to keep every column dense, then fixes up the moved entity's record. Columns never develop holes.

### ChunkLayout — computed packing

`ChunkLayout` is computed once per archetype and shared by all its chunks. `calculate()` sorts the component descriptors by id, then walks them to place each column at a properly aligned offset, shrinking `entityCapacity` in a bounded loop if alignment padding pushes the columns past 16 KB. The result is the per-column byte offsets that `Chunk::getComponentArray<T>()` uses to find a column.

---

## Components

A component is any type satisfying `ValidComponentType`: standard layout and no larger than `max_component_size` (1024 bytes). For a type to be used a component the `LITL_REGISTER_TYPE_NAME` macro must be used on it - typically outside of the owning namespace. Example:

```cpp
namespace game
{
    struct Health
    {
        uint32_t max = 100u;
        uint32_t current = 100u;
    };
}

LITL_REGISTER_TYPE_NAME(game::Health);
```

### Descriptors and the two ids

`ComponentDescriptor::get<T>()` returns a pointer to a function-local `static` descriptor, unique per `T` and stable for the program's lifetime. That descriptor carries:

- **`id` (`ComponentTypeId`, `uint32_t`)** — a runtime id from a monotonic counter. Fast, dense, good for indexing — but **not stable across runs**.
- **`stableId` (`StableComponentTypeId`, `uint64_t`)** — a hash of the type name. Stable across runs — use it for serialization and networking.
- **`size` / `alignment`** — for chunk layout.
- **`build` / `move` / `destroy`** — function pointers that placement-new, move-construct, and destroy a `T` at a given address. This is how chunk code manipulates type-erased component bytes without ever naming `T`.

`stableId` is the reason that `LITL_REGISTER_TYPE_NAME` needs to be called on each prospective component type.

```cpp
build   = [](void* to)              { new (to) T(); };
move    = [](void* from, void* to)  { new (to) T(std::move(*reinterpret_cast<T*>(from))); };
destroy = [](void* ptr)             { reinterpret_cast<T*>(ptr)->~T(); };
```

`ComponentRegistry` keeps the runtime maps (`id → descriptor`, `stableId → descriptor`) for the non-templated paths — e.g. resolving a component that arrived as a runtime id from a command buffer or over the wire.

### ComponentData

`ComponentData { ComponentTypeId type; void* data; }` pairs a component type with a pointer to a source value, for "add and set in one step" operations. It compares and hashes on `type` only and converts implicitly to `ComponentTypeId`, so it slots into the same code paths as a bare id.

---

## Archetypes and movement

`ArchetypeRegistry::get<Foo, Bar>()` resolves (or builds) the archetype for a compile-time set; `getByComponents(...)` does the same from a runtime `ArchetypeComponents` (a fixed `std::array` of up to 64 ids with a lazily-recomputed hash).

Everything structural reduces to a **move**:

```
add component(s):    currentArchetype.components ∪ added   → resolve target → move
remove component(s): currentArchetype.components \ removed → resolve target → move
```

`ArchetypeRegistry::move` transfers the entity's existing columns into the target archetype's chunk (preserving values for components common to both), constructs any newly added components, drops removed ones, swap-removes from the source chunk, and updates the `EntityRecord`. `World::mutateImmediate(entity, add, remove)` is the single primitive that batches an add-set and a remove into **one** move — the deferred command processor leans on it so a frame's worth of changes to an entity costs one archetype transition, not one per component.

The immediate `*Immediate` methods on `World` (`addComponentImmediate`, `removeComponentImmediate`, `mutateImmediate`, …) perform the move synchronously. They're documented as engine/test/demo conveniences — in system code you almost always want the deferred path instead, because a synchronous move invalidates the chunk you're iterating.

---

## Systems

### The update signature is the query

`ValidSystem` requires a `setup(ServiceProvider&)` and an `update(...)` whose first two parameters are exactly `EntityCommands&` and `float` (commands buffer + delta time). Everything after that is the component query, and each must be an lvalue reference:

```cpp
void update(EntityCommands& commands, float dt, Foo const& read, Bar& write);
//                                              ^ read-only      ^ read-write
```

`systemTraits.hpp` pulls this apart at compile time. `SystemComponents<S>` strips the leading two parameters via `SystemTupleTail`; `SystemComponentsTupleOperations` then turns the remaining types into either component ids (for archetype matching) or `SystemComponentInfo { id, readonly }` records. `const&` ⇒ `readonly = true`, `&` ⇒ `readonly = false`. That read/write classification drives implicit scheduling (see below). A `static_assert` rejects by-value or non-reference component parameters with a readable message.

### Type erasure: Wrapper → Runner

A user system type is needed to *build* a `System`, but a `System` stores none of it directly. `System::attach<S>()` constructs a `SystemWrapper<S>` into 64 bytes of inline storage and records three erased function pointers (setup / run / destroy). The wrapper owns the user struct and a `SystemRunner<S>`; the runner is what actually iterates:

```cpp
// SystemRunner<S>::run, per chunk:
auto columns = extractComponentBuffers(chunk, layout);   // tuple<Foo*, Bar*>
for (uint32_t i = 0; i < chunk.size(); ++i)
    std::apply([&](auto&... col){ system->update(commands, dt, col[i]...); }, columns);
```

So storage is SoA, but the user writes an ordinary per-entity `update`. The runner indexes each column in lockstep and expands them into the call.

There is exactly **one** instance of each system, via `SystemRegistry::getSystem<S>()` returning a function-local static. This keeps archetype-match state in one place but assumes a single `World` per process; `SystemManager::~SystemManager` calls `reset()` on each system so test suites that spin up multiple worlds don't leak matched archetypes between them.

### Archetype matching

When new archetypes appear, `SystemManager::prepareFrame` → `updateSystemArchetypes` feeds them to every system's `updateArchetypes(...)`; new systems get the full back-catalog of existing archetypes once. Each system keeps the set of archetypes whose component set is a superset of its query, and at run time iterates the chunks of exactly those.

---

## Scheduling

Systems are partitioned into eight **groups** that run in a fixed order each frame:

```
Startup → Input → FixedUpdate → Update → LateUpdate → PreRender → PostRender → Final
```

Each group owns its own `SystemGraph` — a DAG built from two kinds of edge:

- **Explicit** — `dependsOn<OtherSystem>()` in the builder. Honored only within a group (cross-group ordering is already implied by group order).
- **Implicit** — derived from component access. Two systems conflict if they touch a shared component and at least one writes it (`doesComponentAccessConflict`). The conflict becomes a dependency edge, so a reader is ordered after the writer it shares a component with. This is what makes "Animation reads Transform, Physics writes Transform ⇒ Animation after Physics" fall out automatically.

`SystemPlacementHint::{First, None, Last}` biases ordering within a group before dependencies are applied (a soft "as early/late as possible," not a hard pin). `SystemGraph::build()` applies hints, adds explicit then implicit edges, and topologically sorts the DAG into **layers** — sets of systems with no remaining dependency between them.

At run time (`SystemManager::run(..., JobScheduler&)`), the systems in a layer are dispatched as parallel jobs behind a `JobFence`; the manager waits on the fence, then processes command buffers, then moves to the next layer. So **every layer boundary is a sync point.** (A sequential `run` path exists for tests and is slated for removal.)

---

## Frame loop

`World::run(dt, fixedStep)` drives one frame:

```
invokeFrameStart()
prepareFrame()                       // pick up new archetypes
run Startup, Input
while (accumulated >= fixedStep)     // fixed step: 0..N times per frame
    run FixedUpdate (fixedStep); accumulated -= fixedStep
run Update, LateUpdate, PreRender
invokeRender()                       // render sits between PreRender and PostRender
run PostRender, Final
invokeFrameEnd()
incrementGlobalWorldVersion()        // stamps the next frame's modifications
```

`FrameCallbacks` is the engine's hook surface: `onFrameStart`, `onFrameEnd`, `onRender`, a per-group `onPreGroup`, and `onSyncPoint(group, changes)` — the last fires after each layer's command buffers are processed and carries the `EntityChange` list (see below). The fixed-update accumulator means `FixedUpdate` systems run at a rate decoupled from frame rate: zero times on a fast frame, several on a slow one.

---

## Deferred entity commands

Structural changes — create, destroy, add/remove component, reparent — **cannot** happen mid-system. Systems run in parallel across chunks, with no guaranteed ordering, and a synchronous archetype move would resize and reshuffle the very storage being iterated. So those operations are recorded into an `EntityCommands` buffer and applied at the next sync point.

### Per-thread buffers

`World` seeds one `EntityCommands` per hardware thread and hands each thread its own via `getCommandBuffer()` (indexed by a thread-local id). No locking on the hot path — each worker records into its own buffer. The buffer set grows on demand if more threads appear.

### DeferredEntity

You often want to create an entity *and* configure it before it exists. `EntityCommands::createEntity()` returns a `DeferredEntity`: a versionless index local to that command buffer. You can `addComponent`/`removeComponent`/`setParent` against it just like a real `Entity`. At the sync point it is **materialized** into a real `Entity`, and the deferred commands referencing it are rewritten to target the materialized id. A `DeferredEntity` is invisible to all other systems until materialized.

`addComponent` overloads cover three data-ownership stories: id only (no value), `void* sharedData` (caller keeps the value alive until the command runs), and `localData + size + alignment` (copied into the buffer's internal pool immediately). The templated `addComponent(entity, T value)` uses the copy path.

### Processing: combine, sort, batch

`EntityCommandProcessor::process` runs at each sync point:

1. **Materialize** deferred entities and **combine** every thread's commands into one vector.
2. **Sort** by `(entity.index, command type, component id)`. The `EntityCommandType` enum values are ordered (`Create < Destroy < Add < Remove < SetParent`) precisely so the sort groups all of one entity's commands together in a sensible order.
3. **Walk** the sorted commands per entity, accumulating adds/removes, then flush them through **one** `mutateImmediate` — a single archetype move per entity. A `Destroy` short-circuits the rest of that entity's commands; `Create` and `SetParent` emit change records.
4. Emit an `EntityChange` list (`{ type, entity, prevArchetype, currArchetype, parent }`) delivered to `onSyncPoint`, then **reset** every buffer (pools kept, offsets rewound).

So a chaotic, multi-threaded stream of fine-grained requests collapses into at most one structural move per entity per sync point, applied single-threaded and deterministically.

### Reparenting

Hierarchy changes ride the same rails. `ParentEntity` wraps a parent `Entity` behind a `ParentEntityWriteKey` passkey so it can't be reassigned directly inside a system — reparenting is structural and must go through a `setParent` command. The processor only *emits* the `SetParent` change; the actual hierarchy fix-up is consumed downstream (by a scene-level processor outside `litl-ecs`).

---

## Conventions and invariants

### Components are dumb data

A component is standard-layout and ≤ 1024 bytes, with no logic. Behavior lives in systems. Keep components small — the smaller the component, the more entities pack into a chunk and the better iteration scales.

### Immediate vs deferred

The `*Immediate` `World` methods mutate synchronously and are for setup, tests, and internal engine use. Anything happening *during* the frame's system execution must use an `EntityCommands` buffer, or it will corrupt in-flight iteration.

### `getComponent` returns a copy

`World::getComponent<T>(entity)` returns `std::optional<T>` — a **copy**. Mutating it does nothing to chunk storage; use `setComponent`. This is intentional: an out-of-system read may happen at an unsafe time, so writes are made deliberate rather than implicit. Reading or writing a component this way at all is unusual — the normal path is iterating it in a system.

### One system instance, one world

Each system is a process-wide static. Multiple `World`s in one process (i.e. tests) share those instances and rely on `SystemManager` teardown to reset matched-archetype state. Production assumes a single world.

### World version

A global version counter increments once per frame and is stamped into a chunk's header on modification. It's the substrate for future change-detection queries; nothing consumes it for filtering yet.

---

## Common patterns

### A read-only system

```cpp
struct RenderExtractSystem
{
    void setup(ServiceProvider&) {}
    void prepare() {}
    void update(EntityCommands&, float, Entity e, Transform const& t, MeshRef const& m)
    {
        // const& everywhere ⇒ no write conflicts ⇒ runs parallel to other readers
    }
};
world.getSystemCollection()
     .addSystem<RenderExtractSystem>(SystemGroup::PreRender);
```

### Ordering systems explicitly

```cpp
world.getSystemCollection()
     .addSystem<AnimationSystem>(SystemGroup::Update)
     .dependsOn<PhysicsSystem>()        // explicit edge
     .placement(SystemPlacementHint::Last);
```

(The implicit edge would already order Animation after Physics if they share a written component — the explicit `dependsOn` is for ordering that *isn't* expressible through component access.)

### Spawning a configured entity from inside a system

```cpp
void update(EntityCommands& commands, float dt, Spawner& s)
{
    DeferredEntity e = commands.createEntity();
    commands.addComponent(e, Position{ s.x, s.y, s.z });
    commands.addComponent(e, Velocity{ 0, -1, 0 });
    // Materialized into a real Entity at the next sync point.
}
```

### Destroying an entity from a command buffer

```cpp
// Entity is not an update() query parameter — destruction is typically driven
// from gameplay code holding the entity, or from a component carrying its own id.
commands.destroyEntity(entity);   // recorded now, applied at the next sync point
```

---

## What's not yet here

Gaps worth knowing about, for context on the current shape:

- **No standalone query API.** Systems are the only iteration path; there's no `world.query<Foo, Bar>()` view object for ad-hoc traversal outside a system (`getComponentMask` is stubbed/commented).
- **Change detection unused.** Chunk `version` is written but no query consumes it to skip unchanged chunks.
- **Runtime system lookup.** `SystemRegistry::getSystem(SystemTypeId)` returns `nullptr` — only the templated lookup works.
- **Single-world assumption.** Static system instances mean one world per process outside tests.
- **Entity cap not enforced.** Indices can grow to `2^32` with no configurable ceiling yet (the tracking structures grow first).
- **Hierarchy resolution lives outside the library.** `litl-ecs` emits `SetParent` changes; applying them is a scene-layer concern.
- **Sequential scheduler path** (`SystemGraph::run(..., vector<System*>)`) is test-only and slated for removal in favor of the job-based path.

Each is a deliberate deferral; none is locked out by the current structure.

---

## Useful files to read

When the document is no longer enough, these are the load-bearing files:

| File | What lives here |
|------|-----------------|
| `litl/ecs/include/litl-ecs/world.hpp` | `World` front door — immediate ops, `run`, command-buffer access |
| `litl/ecs/include/litl-ecs/constants.hpp` | All sizing constants and id typedefs; `ValidComponentType` |
| `litl/ecs/include/litl-ecs/entity/entity.hpp` | `Entity` index+version rationale |
| `litl/ecs/include/litl-ecs/component/component.hpp` | `ComponentDescriptor`, the build/move/destroy table, id generation |
| `litl/ecs/include/litl-ecs/archetype/chunk.hpp` | Chunk layout, column access, `removeAndSwap` |
| `litl/ecs/src/litl-ecs/archetype/chunkLayout.cpp` | The column-packing / capacity calculation |
| `litl/ecs/include/litl-ecs/system/systemTraits.hpp` | `ValidSystem`, signature decomposition, read/write extraction |
| `litl/ecs/include/litl-ecs/system/systemRunner.hpp` | Per-chunk iteration over SoA columns |
| `litl/ecs/src/litl-ecs/system/systemManager.cpp` | Group running, layer fences, sync points |
| `litl/ecs/include/litl-ecs/system/systemGraph.hpp` | DAG, explicit + implicit dependencies, layers |
| `litl/ecs/src/litl-ecs/entity/entityCommandProcessor.cpp` | Combine / sort / batch-mutate at sync points |
| `litl/ecs/src/litl-ecs/world.cpp` | Frame loop and the immediate archetype-move operations |
| `tests/src/litl-ecs/world_tests.cpp`, `system_tests.cpp` | Working examples of every path |

The tests are the most accurate documentation — they're the paths through the API known to compile and pass.
