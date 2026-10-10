# LITL Assets & Import — Design Reference

A high-level overview of the asset system in `litl-engine` (`litl/engine/include/litl-engine/assets`) and the format-conversion library underneath it, `litl-import`.

## Overview

Two layers cooperate to turn files on disk into engine objects (meshes on the GPU, materials, textures in the bindless table, …):

- **`litl-import`** — a standalone static library that converts *external* formats (`.glb`, `.obj`, `.png`, `.slang`, `.litlmat`, …) into a small set of **intermediate** representations, and serializes those intermediates to LITL's **internal** binary formats (`.litlbmsh`, `.litlbtex`, …). It knows nothing about the engine, the renderer, or threads.
- **The asset system** (`litl-engine`) — discovers what assets exist, maps string keys to handles, and loads them asynchronously: bytes are read and decoded on a worker thread, dependencies are awaited, and GPU-facing work happens on the main thread. It calls into `litl-import` whenever it meets an external format.

The pieces, from the bottom up:

- **`ImportService`** — the front door of `litl-import`. Owns an `ImporterRegistry` (source type → `Importer`) and an `ExporterRegistry` (intermediate type → `Exporter`).
- **`ImportedData`** — the output of an import: a list of typed items (mesh, material, model, shader, texture). One source file can produce many items.
- **`AssetSource`** — where asset bytes come from. `FileAssetSource` (loose files under `assets/`) is the only implementation today.
- **`AssetRegistration`** — what a source reports for each asset it can provide: key, type, source format, priority, locator.
- **`Asset`** (`MeshAsset`, `MaterialAsset`, …) — per-asset load state plus an `AssetOps` function table that defines how that type decodes and finalizes.
- **`AssetManager`** — the engine service. Owns the registrations, the per-type asset pools, the load scheduling, and the dependency waiters.
- **`AssetLoadTask`** — the coroutine that drives one asset from `Loading` to `InMemory` or `Error`.

The governing idea: **assets are addressed by key, loaded lazily on first access, and never block the caller.** A `getX(...)` call returns immediately with an asset whose `status` is `Loading`; callers (typically systems) poll the status until it settles.

---

## Quick start — getting assets

**Keys.** An asset's key is its path relative to `assets/`, lowercased, with the extension removed. `assets/materials/lit.litlmat` has the key `materials/lit`; `assets/models/cube.glb` has the key `models/cube`. Lookups are case-insensitive.

**Fetching an asset:**

```cpp
auto assets = services.get<AssetManager>();

// Handle lookup only. Does not trigger a load.
MaterialAssetHandle handle = assets->getMaterialHandle("materials/lit");

// Fetching the asset by handle (or key) triggers the load if it is still Unloaded.
MaterialAsset* material = assets->getMaterial(handle);

// Loading is asynchronous. Poll until it settles.
switch (assets->getMaterialAssetStatus(handle))
{
case AssetStatus::Loading:  /* try again next frame */      break;
case AssetStatus::InMemory: /* material->material is usable */ break;
case AssetStatus::Error:    /* material->error says why */  break;
default: break;
}
```

**Spawning a model into the world.** Models are not instantiated by hand. Attach a `PendingModelInstance` and the engine's `ModelInstantiationSystem` takes it from there:

```cpp
const auto entity = commands.createEntity();
commands.addComponent<Transform>(entity, Transform{});
commands.addComponent<LocalBounds>(entity, LocalBounds{});
commands.addComponent<PendingModelInstance>(entity, PendingModelInstance{
    .modelHandle = assets->getModelHandle("models/cube"),
    .fallbackMaterialHandle = fallbackMaterial              // used for meshes with no (or a failed) material
});
```

Each frame the system checks the model's status. On `Unloaded` it triggers the load; on `InMemory` it creates one child entity per model node (with `Transform`, `MeshRef`, `LocalBounds`, and a `MaterialRef` or `VariableMaterialsRef`) and swaps `PendingModelInstance` for `ModelInstance`; on `Error` it swaps in `FailedModelInstance`.

---

## Discovery and registration

### FileAssetSource

At `AssetManager::setup`, each `AssetSource` **enumerates** everything it can provide. `FileAssetSource("assets")` walks the directory recursively and registers every file whose extension it recognizes:

| Asset type | Extension | Format | Priority |
|------------|-----------|--------|----------|
| Material | `.litlmat` (TOML) | Internal | High |
| Material | `.litlbmat` | Internal | Medium |
| Mesh | `.litlbmsh` | Internal | High |
| Model | `.litlmdl` (JSON) | Internal | High |
| Model | `.glb` | External | Medium |
| Model | `.obj` | External | Low |
| Shader | `.litlbshd` | Internal | High |
| Shader | `.spv` | External | Medium |
| Shader | `.slang` | External | Low |
| Text | `.txt`, `.json` | External | Medium |
| Texture | `.litlbtex` | Internal | High |
| Texture | `.bmp`, `.jpg`/`.jpeg`, `.png`, `.tga` | External | Medium |

A source also implements `read(locator, bytes)` (called on a worker thread), `describe(locator)` (for logs), and `resolve(base, reference, outLocator)`, which turns a relative reference found inside one asset (e.g. an `.obj`'s `mtllib`) into a locator. `FileAssetSource::resolve` resolves against the base file's folder and appends unknown files to its table on demand.

### Key collisions and priority

Keys omit the extension, so `models/cube.glb` and `models/cube.obj` both claim `models/cube`. The `AssetManager` keeps **only the highest-priority registration per key** and logs a warning for the loser; on a tie, the first one enumerated wins. Note that this comparison ignores asset type: a `.litlbmsh` and an `.obj` with the same stem compete for one key even though one is a Mesh and the other a Model.

Priority currently comes only from the extension table above but will in the future also be weighted by source (e.g. bundles over loose files in release builds).

### Handles and pools

For every surviving registration the manager creates a placeholder asset in the `Unloaded` state, in a per-type `LockedHandlePool`, and stores its `AssetHandle` (a tagged union of the six typed handles) in the registration. So **every asset that exists on disk has a valid handle from startup**, whether or not it has ever been loaded. `getAsset(key)` returns that `AssetHandle`; the typed `getXHandle(key)` returns the typed handle or an invalid one if the key belongs to a different type.

When a load starts, the asset reserves its engine object in the `ObjectPool` (`Mesh`, `Material`, `Texture`, `Shader`, `Text`) and binds it via `fetchAssetObject`, so `asset->mesh` / `->material` / … exist before any bytes are read.

---

## The load pipeline

### Triggering a load

`getX(handle)` / `getX(key)` check the asset's status and, if `Unloaded`, CAS it to `Loading`, reserve and fetch its engine object, and schedule an `AssetLoadTask` coroutine on the `TaskManager`. The CAS makes concurrent first accesses safe: exactly one caller starts the load, everyone gets the same asset pointer back.

### loadFromDiskAsync

```
[calling thread]  validate the AssetOps table (decodeAssetBytes is required)
        │
co_await ResumeTaskOnWorkerThread
        │
[worker]  source->read(locator)                         → bytes
          scanExternalDependencies (optional)           → e.g. .obj → [.mtl]
              for each: source->resolve + source->read  → ImportCompanion{ reference, bytes }
          decodeAssetBytes(bytes, companions)           → asset-specific intermediate data
          processOnWorker (optional)
        │
co_await ResumeTaskOnMainThread
        │
[main]    gatherAssetDependencies (optional)            → other Assets this one needs
          co_await AwaitAssetDependencies                (suspends until all settle)
          processOnMain (optional)                      → GPU upload, object creation
          status = InMemory   (or Error at any failed step)
```

Every step checks `status` first, so the first failure short-circuits the rest and its `AssetErrorCode` is kept (`setError(err, default)` only fills in a default if the step didn't set something more specific).

**External dependencies vs asset dependencies.** These are two different things:

- *External dependencies* (`scanExternalDependencies`) are companion **files** that aren't assets in their own right — an `.obj`'s `.mtl`, a `.gltf`'s `.bin`. Their bytes are read on the worker and handed to the importer as `ImportCompanion`s. If one can't be resolved or read, the load continues unless the registration's `ImportSettings::continueOnDependencyFailure` is `false` (it defaults to `true`).
- *Asset dependencies* (`gatherAssetDependencies`) are other **assets** — a material's shaders and textures, a model's meshes and materials. Gathering them triggers their loads (via `getX`), and the dependent then awaits them.

### Awaiting dependencies

`AwaitAssetDependencies` is a coroutine awaitable. If every dependency is already `InMemory` or `Error`, it doesn't suspend. Otherwise the coroutine handle is parked in the manager's pending list (`registerAwaitingDependency`). `AssetManager::onFrameStart` — called from the engine's `onFrameStart` callback — re-checks every parked waiter and schedules the ready ones onto the main-thread task queue. A dependency wait therefore costs at least one frame.

On resume, if any dependency ended in `Error`, the dependent fails with `DependencyLoadFailed` **only if** its `AssetOps::requiresAllDependencies` is set. Materials, meshes, and shaders require everything; models don't, so a model with one broken mesh still loads and that mesh's handle is simply cleared in `processOnMain`.

### loadFromMemoryAsync

Assets created from data already in memory (see [Models](#models)) skip the read/scan/decode stage: the intermediate data is attached directly, and the task runs only `processOnWorker` (if present) and the shared main-thread stage.

---

## Asset types

Each type defines its behavior through an `inline constexpr Asset::AssetOps` table. Every type accepts its internal format directly and falls back to `ImportService::importForMemory` for external formats, logging a warning that recommends converting to the internal format.

| Type | Decode produces | Gathers | `processOnMain` |
|------|-----------------|---------|-----------------|
| `TextAsset` | the `Text` object directly | — | — |
| `ShaderAsset` | `ShaderIntermediateData` (SPIR-V + reflection) | — | creates the renderer shader module |
| `TextureAsset` | `TextureIntermediateData` | — | builds a `TextureResourceDescriptor` (in the texture table by default), uploads pixels |
| `MeshAsset` | `GeoMesh` (into the `Mesh` object) | — | records bounds, uploads vertex/index buffers |
| `MaterialAsset` | `MaterialIntermediateData` | shader assets per stage; texture assets per `texture` property | `Material::setData` with the resolved shaders |
| `ModelAsset` | `ModelIntermediateData` (+ the full `ImportedData` when external) | creates in-memory mesh/material/texture assets | clears handles of failed sub-assets |

Intermediate data is dropped once `processOnMain` has consumed it; only the engine object (and, for models, the handle tables) remains.

### Materials

A material names its shaders by asset key and lists typed properties. The hand-authored form is TOML (`.litlmat`):

```toml
name = "Lit"

[shaders]
vertex   = { resource = "shaders/lit", entry = "vertexMain" }
fragment = { resource = "shaders/lit", entry = "fragmentMain" }

[raster]
cullMode  = "back"
frontFace = "clockwise"

[properties]
tint      = { type = "color", value = [1.0, 1.0, 1.0, 1.0] }
roughness = { type = "float", value = 0.5 }
baseColor = { type = "texture", value = "white" }

[hints]
frequentUpdates = false
```

Stage names accept several aliases (`vert`/`vertex`, `frag`/`fragment`, `hull`/`tessControl`, `domain`/`tessEval`, `compute`, `mesh`, `task`, …), as do property types (`int`/`int32`/`integer`, `vec3`/`float3`, …).

A `texture` property's value is a **texture asset key** (e.g. `textures/brick`). During `gatherAssetDependencies` each key is looked up and becomes a dependency. When the material is built, the texture's bindless table slot is packed with a sampler index (`LinearRepeat` by default) into the property's `uint`. If no texture asset has that key, the reserved names `pink`, `white`, `black`, and `normal` map straight to the reserved texture-table slots; any other unresolved key logs a warning and leaves the property unset. See [`renderer.md`](./renderer.md#bindless-textures).

A `.slang` shader referenced by a material is compiled on first load, which is the slowest part of the first frame when nothing has been pre-converted.

### Models

A model is **not** a container of data; it's a description of a scene fragment: a list of mesh names, a list of material names, and a node hierarchy (`Node { name, localTransform, meshIndex, materialIndices[], children[] }` plus `rootNodes`).

When a model is loaded from an external format, the importer has already produced the meshes, materials, and textures as sibling items in the same `ImportedData`. `ModelAsset::gatherAssetDependencies` turns each into **an asset of its own, created from memory**, keyed under the model:

```
models/sponza                       ← the ModelAsset
models/sponza/<mesh name>           ← MeshAsset      (createMeshAssetFromMemory)
models/sponza/<material name>       ← MaterialAsset  (createMaterialAssetFromMemory)
models/sponza/<image name>          ← TextureAsset   (createTextureAssetFromMemory)
```

Embedded textures are linked to materials through the importer's `MaterialTextureLink { materialItemIndex, textureItemIndex, propertyName }` records. The texture asset is created first and its key is written into the material's property, so the material then gathers it like any other texture dependency. Item names have already been sanitized and deduplicated by the importer (see [Item naming](#item-naming)), which is what makes these keys unique.

The `createXAssetFromMemory` functions take an `Authority<ModelAsset>` passkey. They register a new key on the fly (under the registrations mutex, since this races with readers) and return the existing handle if the key is already taken by the same type.

---

## litl-import

### Importers and exporters

Each object type (material, mesh, model, shader, texture) has three parts under `litl/import/{include,src}/litl-import/<type>/`:

- **`import/`** — format-specific `Importer`s that produce the intermediate representation.
- **`intermediate/`** — the intermediate type and its binary (de)serializer.
- **`export/`** — a single `Exporter` for that intermediate type.

```cpp
class Importer
{
    virtual Result import(ImportContext const& context, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept = 0;
    virtual Result scanDependencies(std::string_view location, std::span<std::byte const> sourceBytes,
                                    ImportSettings const& settings, std::vector<ImportDependency>& outDependencies) noexcept;  // default: none
};

class Exporter
{
    virtual Result prepare(ImportedData& data, ImportSettings const& settings, uint32_t dataIndex) noexcept = 0;
    virtual Result write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept = 0;
};
```

Concrete types are registered in `ImportService::registerProcessors` through `ValidImporter` / `ValidExporter` concepts, which require static metadata (`ImporterName` + `SupportedTypes`; `ExporterName` + `OperatesOnImportedDataType` + `ExportedExtension`). The registries store factory functions, and a fresh importer/exporter is created per call — they're stateless from the service's point of view.

| Importer | Source type(s) | Produces |
|----------|----------------|----------|
| `GlbImporter` (cgltf) | `ModelGlb` | 1 model + N meshes + N materials + N textures |
| `ObjImporter` (rapidobj) | `ModelObj` | 1 model + meshes + materials (textures referenced by key) |
| `LitlMatImporter` (glaze TOML) | `MaterialLitl` | 1 material |
| `SlangImporter` | `ShaderSlang` | 1 shader (compiled to SPIR-V 1.6) |
| `SpirvImporter` | `ShaderSpirv` | 1 shader |
| `PngImporter`, `JpegImporter`, `BmpImporter`, `TgaImporter` (stb_image) | `Texture*` | 1 texture |

### ImportedData

```cpp
struct ImportedData
{
    std::vector<ImportedDataItem> items;   // each: a name + std::variant of unique_ptr<XImportResult>
    ImportedDataResult result;             // first failure + its item index, for multi-item operations
};
```

An `ImportedDataItem` holds exactly one of `MaterialImportResult`, `MeshImportResult`, `ModelImportResult`, `ShaderImportResult`, `TextureImportResult`. Its type is set once with `setType` and can't change. The variant's alternative indices are pinned to the `ImportedDataType` enum by `static_assert`s.

Items reference each other **by index** within the same `ImportedData` — a `ModelImportResult` carries `ModelDataItem { importedDataItemIndex, modelNameIndex }` for its meshes/materials and `MaterialTextureLink`s for textures. This is why an importer appends to `items` rather than replacing it.

### Import flow

`ImportService` offers two entry points:

- **`importForMemory(sourceType, location, bytes, settings, companions, out, shouldPrepare)`** — create the importer for `sourceType`, run it, sanitize and deduplicate item names, and (if `shouldPrepare`) run each item's exporter `prepare`. This is what the asset system uses (always with `shouldPrepare = true`).
- **`importForWriting(...)`** — `importForMemory` without prepare, then `prepare` + `write` each item into `WriteableImportResults::bytes[i]`. This produces internal-format file contents. Nothing in the engine calls it yet; it's exercised by the import tests and intended for an offline conversion step.

`scanForDependencies(sourceType, location, bytes, settings, out)` asks the importer which companion files it needs. Only `ObjImporter` overrides it (each `mtllib` line → a non-required `MaterialLibrary` dependency).

### Prepare: normalizing to engine conventions

`prepare` is where an intermediate is brought into the engine's conventions, regardless of source format:

- **Meshes** (`MeshExporter`): triangulate, shrink, convert counter-clockwise winding to clockwise, generate normals if missing, negate Z for right-handed sources, flip V if requested, finalize submeshes. Each importer records its source's conventions in `MeshImportConvention { sourceIsRightHanded, sourceIsCcwFront, flipTexcoordV }` — glTF and OBJ are both right-handed CCW, and OBJ also flips V.
- **Shaders** (`ShaderExporter`): run SPIR-V reflection and store it in the intermediate, so the engine never re-reflects at load.
- **Materials, textures, models**: validation only today.

### Embedded imports

Some containers carry other formats inside them (a `.glb` carries PNG/JPEG images). An importer can recurse through `ImportContext::embeddedImporter.importEmbedded(sourceType, name, bytes, settings)`, which runs the nested importer into the **same** `ImportedData` and returns the new item's index. Only texture types are allowed to be embedded, which keeps nesting one level deep. The GLB importer forces `ColorTextureImportSettings` (sRGB, mipmaps) for embedded images, since only base-color textures are linked today.

### Item naming

After import, `sanitizeAndDeduplicateImportedItemNames` makes every item name safe as both a file name and an asset-key segment:

1. Lowercase and sanitize; a reserved file name (e.g. `con`) is cleared.
2. Unnamed items get `<type>_<n>` (`mesh_0`, `material_1`, …).
3. Duplicates get a suffix (`wall`, `wall_1`, `wall_2`, …).
4. `propagateNameUpdates` pushes the final names back into the model's mesh/material name lists.

### Textures

All texture importers go through stb_image, forced to 4 channels. `TextureIntermediateData` stores pixels as **RGBA32 float**, top-left origin, with a `TextureLevel` table for the mip chain. If the settings' transfer function is sRGB, pixels are decoded to linear on import. If mipmaps are enabled, the full chain is generated **on the CPU** with a 2×2 box filter (`generateMipMaps`; float, single-layer 2D only). `TextureImportSettings` presets exist for color (`ColorTextureImportSettings`: sRGB + mips), normal maps, and masks (both linear, no mips).

### Shaders

`SlangImporter` keeps a thread-local Slang session targeting SPIR-V 1.6 with column-major matrices. The session's module search path is `assets/shaders` (relative to the working directory), which is how shaders `import litl.core;` — the engine-provided modules live in `assets/shaders/litl/`.

### Internal formats

All internal binary formats share `BinaryBlockFile` (in `litl-core`): a header (4-char magic + major/minor version), a table of block descriptors, then 16-byte-aligned blocks identified by 4-char ids, with a shared `STRS` string block. Parsing validates sizes, offsets, overlap, versions, and a content hash before any block is read.

| Format | Magic | Blocks | Intermediate |
|--------|-------|--------|--------------|
| `.litlbmsh` | `LMSH` | `VRTX`, `INDX`, `FACE` (omitted if all triangles), `BNDS`, `SUBM`, `MTRL` | `GeoMesh` |
| `.litlbtex` | `LTEX` | `INFO`, `MIPS`, `PIXL` | `TextureIntermediateData` |
| `.litlbmat` | `LMAT` | `SHDR`, `PROP`, `SETT` | `MaterialIntermediateData` |
| `.litlbshd` | `LSHD` | `ENTR`, `RESB`, `PUSH`, `PURP`, `VFIO`, `RESP`, `SPEC`, `SPRV`, `META` | `ShaderIntermediateData` |
| `.litlmdl` | — (JSON) | — | `ModelIntermediateData` |

`.litlmdl` is deliberately JSON so it's readable and hand-checkable; a model holds names and transforms, not bulk data.

---

## Conventions and invariants

### Keys are the identity

An asset is identified by its lowercased, extension-less path under `assets/`. Sub-assets created from a model live under the model's key. Code should hold handles, not keys, once it has them; key lookups take the registrations mutex.

### Never block on an asset

There is no synchronous load. `getX` starts a load and returns; anything that needs the result polls the status (as `ModelInstantiationSystem` does) or is itself an asset that declares a dependency.

### Worker vs main thread

Reading, scanning, decoding, and importing run on worker threads and must touch only the asset's own intermediate data. Anything that touches the renderer, the `ObjectPool`, or other assets runs in `gatherAssetDependencies` / `processOnMain` on the main thread.

### Intermediate data is transient

Once `processOnMain` succeeds, the intermediate data is released. The loaded asset is its engine object (`Mesh`, `Material`, `Texture`, …), not the intermediate.

### Engine conventions are applied in `prepare`

Importers record what their source looks like (handedness, winding, V origin, color space); exporters' `prepare` converts it. Engine code downstream of `litl-import` can assume left-handed, clockwise, top-left-origin, linear-color data.

---

## What's not yet here

Gaps worth knowing about, for context on the current shape:

- **`.litlmdl` models can't load.** `ModelAsset::gatherAssetDependenciesFromLitlModel` is a `todo` that returns `false`, so a `.litlmdl` asset ends in `DependencyResolveFailed`. Only external-format models (`.glb`, `.obj`) load today.
- **No offline conversion step.** `importForWriting` exists, but nothing writes internal-format files for `assets/`; every external asset is re-imported on each run (with a warning).
- **No per-asset import settings.** `FileAssetSource` leaves `AssetRegistration::importSettings` at defaults, so a standalone `.png` loads as linear with no mips. Only GLB-embedded textures get the color preset. A sidecar or naming convention for settings doesn't exist yet.
- **Textures are uncompressed RGBA32F.** 16 bytes per texel; BC encoding is a `todo` in `TextureExporter::prepare`.
- **Mesh processing is minimal.** Welding, degenerate removal, tangent generation (MikkTSpace), vertex-cache/overdraw optimization, and LODs are all `todo` in `MeshExporter::prepare`.
- **GLB materials are partial.** Base color, tint, roughness, and metallic factors are imported; normal/metallic-roughness/occlusion/emissive maps, alpha modes, and alpha cutoff are not. Default shaders (`shaders/lit`) are hardcoded in the importer.
- **No stall detection for dependencies.** A dependency that never leaves `Loading` keeps its dependent parked forever (`framesPending` is counted but unused).
- **Key collisions ignore type.** Priority resolution happens per key across all asset types.
- **One asset source.** No bundles, no project-specific asset roots, no hot reload of assets.
- **Slang search path is CWD-relative.** Shaders only resolve their `litl/` modules when the process runs from the repository (or an equivalent layout).
- **No unloading.** Assets move `Unloaded → Loading → InMemory/Error` and stay there.

Each is a deliberate deferral; none is locked out by the current structure.

---

## Useful files to read

When the document is no longer enough, these are the load-bearing files:

| File | What lives here |
|------|-----------------|
| `litl/engine/include/litl-engine/assets/assetManager.hpp` | Public asset API: handles, typed getters, from-memory creation |
| `litl/engine/src/assets/assetManager.cpp` | Registration, priority resolution, lazy load triggering, dependency waiters |
| `litl/engine/src/assets/assetLoadTask.cpp` | The disk/memory load coroutines and their thread hops |
| `litl/engine/include/litl-engine/assets/asset.hpp` | `Asset` base and the `AssetOps` table |
| `litl/engine/src/assets/fileAssetSource.cpp` | Extension → type/priority table, key derivation, reference resolution |
| `litl/engine/src/assets/modelAsset.cpp` | Turning one imported model into mesh/material/texture sub-assets |
| `litl/engine/src/assets/materialAsset.cpp` | Shader and texture dependency gathering |
| `litl/engine/src/ecs/systems/modelInstantiationSystem.cpp` | `PendingModelInstance` → entity hierarchy |
| `litl/import/include/litl-import/importService.hpp` | `importForMemory` / `importForWriting` / `scanForDependencies` |
| `litl/import/include/litl-import/importedData.hpp` | `ImportedData`, item types, the variant |
| `litl/import/src/model/import/glb.cpp` | The most complete importer: model, meshes, materials, embedded textures |
| `litl/import/src/mesh/export/meshExporter.cpp` | Convention normalization for meshes |
| `litl/import/src/texture/intermediate/textureIntermediateData.cpp` | Float conversion, sRGB decode, CPU mip generation |
| `litl/core/include/litl-core/formats/binaryBlockFile.hpp` | The shared internal binary container |
| `tests/src/litl-import/*`, `tests/src/litl-engine/asset_tests.cpp` | Working examples of every importer and the asset paths |

For how loaded textures reach shaders, see [`renderer.md`](./renderer.md#bindless-textures); for how instantiated model entities enter the scene, see [`scene.md`](./scene.md).
