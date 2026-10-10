# LITL Renderer — Design Reference

A high-level overview of `litl-renderer` and `litl-renderer-vulkan`.

## Overview

The renderer is split into two layers:

- **`litl-renderer`** — a backend-agnostic API. Declares all the public types, descriptors, the `Renderer` wrapper class, and a function-pointer table (`RendererOps`). Knows nothing about Vulkan, D3D12, Metal, or anything else. User code includes only these headers.
- **`litl-renderer-vulkan`** — the Vulkan 1.4 implementation. Defines concrete versions of every opaque type and fills in every function in the ops table. The user instantiates a renderer via `createVulkanRenderer(...)` and from then on talks only to the abstract API.

The split exists for two reasons. First, replaceability: a future D3D12 or Metal backend slots in by providing its own `createXyzRenderer` and `RendererOps` implementation, with no user code changes. Second, compile isolation: heavy Vulkan headers stay out of user translation units.

The boundary is enforced by a function-pointer table plus an opaque context pointer. See [Architecture](#architecture).

---

## Quick start — the minimal render loop

```cpp
#include "litl-renderer/renderer.hpp"
#include "litl-renderer-vulkan/integration.hpp"

using namespace litl;

Window*   window   = createVulkanWindow();
window->open("My Sample", 1024, 768);

Renderer* renderer = createVulkanRenderer(window, RendererConfiguration{
    .rendererType  = RendererBackendType::Vulkan,
    .framesInFlight = 2,
});
renderer->build();

while (!window->shouldClose())
{
    if (renderer->beginRender())
    {
        auto cb = renderer->cmdBeginFrame();

        renderer->cmdPipelineBarrier(cb, PipelineBarrierUndefinedToColor);
        renderer->cmdBeginRender(cb, BeginRenderCommand{                    // also binds the global texture table at set 0
            .color = { .clearColor = color(0.05f, 0.05f, 0.075f, 1.0f) }
        });
        renderer->cmdSetViewportAndScissor(cb, /* normalized full-screen */);

        // ... your draws ...

        renderer->cmdEndRender(cb);
        renderer->cmdPipelineBarrier(cb, PipelineBarrierColorToPresent);
        renderer->cmdEnd(cb);

        renderer->submitCommands(cb);
        renderer->endRender();
    }
}
```

The pattern: `beginRender → cmdBeginFrame → record → cmdEnd → submitCommands → endRender`. Everything else is recording calls between begin and end.

---

## Architecture

### The FFI boundary

`litl-renderer/renderer.hpp` forward-declares `struct RendererContext` without ever defining it. The `Renderer` class holds a `RendererContext*` and a `RendererOps const*` (the function-pointer table). Every public method on `Renderer` reads as:

```cpp
void Renderer::cmdDraw(CommandBufferHandle cb, uint32_t v, uint32_t i, uint32_t fv, uint32_t fi) const noexcept
{
    m_pOps->cmdDraw(m_pContext, cb, v, i, fv, fi);
}
```

On the backend side, `litl-renderer-vulkan/rendererContext.hpp` declares `litl::vulkan::RendererContext`, the *real* struct. It holds `VkInstance`, `VkDevice`, the swapchain, the resource manager, per-frame sync info, all of it. The two `RendererContext` types live in different namespaces and are unrelated by C++ rules — the casts that bridge them go through `unwrap()` / `wrap()` helpers using `reinterpret_cast`.

This works because user code never *defines* the abstract `RendererContext` — it's always passed by pointer. The pointer's bit pattern is whatever the backend chose. The function table dispatches to backend code that knows how to interpret it.

### Function table dispatch

`RendererOps` is a struct of function pointers. `createVulkanRenderer` builds one of these (statically — it's a constexpr-shaped table in `litl-renderer-vulkan/renderer.hpp`) and hands it to the `Renderer` constructor. Every call routes through one indirection.

The cost is a function-pointer call per op — negligible compared to whatever Vulkan does next. The benefit is that swapping backends is a constructor-level change.

### Resource handles

Resources cross the boundary as typed integer handles, not pointers. `BufferHandle`, `TextureResourceHandle`, `ShaderModuleHandle`, etc. are thin wrappers around a generation+index pair (`Handle<Tag>`). Storage lives backend-side in `HandlePool<Resource, Tag>`s; user code never sees a `VkBuffer`.

Handles survive backend resource churn (hot reload, recreation) because the *handle* is stable while the *underlying resource* gets rebuilt in place. See [Shader hot reload](#shader-hot-reload).

### Device requirements

The Vulkan backend requires a 1.4 device. `doesPhysicalDeviceSupportRequiredFeatures` rejects any physical device lacking one of the features `createRequiredFeaturesChain` enables, including: push descriptors, `synchronization2`, dynamic rendering, extended dynamic state, buffer device address, descriptor indexing (runtime descriptor arrays, partially-bound, sampled-image update-after-bind, non-uniform sampled-image indexing), shader draw parameters, sampler anisotropy, `shaderInt64`, and BC texture compression. The two functions must be kept in sync when a feature is added.

### Build order

Global resources are built in a fixed order, because each depends on the one before:

```
ResourceManager::buildEarly   → SamplerCache
SamplerArray::build           → the 16 predefined samplers (via SamplerCache)
TextureTable::build           → global bindless set layout + set (references the SamplerArray)
ResourceManager::buildLate    → PipelineLayoutCache (needs the TextureTable's set layout)
```

---

## Frame loop and synchronization

### Frames in flight

The renderer keeps `framesInFlight` slots (`RendererConfiguration` suggests 1–2; `RendererConstants::MaxFramesInFlight` is 4). Each slot has its own:

- **Command buffer** — recorded into during the frame, submitted at frame end.
- **Render fence** — signals when the GPU is done with this slot's work.
- **Semaphores** — image-acquire and render-complete for ordering swap.
- **Staging arenas** (buffer + texture) — per-frame transfer scratch space.
- **Descriptor set allocator** — pool used for transient descriptor allocations this frame.
- **Destruction queue** — resources whose destruction was deferred while this slot was current.
- **Depth texture** — the swapchain-sized depth attachment for this slot.

The slot index cycles every frame: `slot = frameCount % framesInFlight`. With two slots, by the time we return to slot 0 again, the GPU has had two frames' worth of time to finish slot 0's previous work.

### The "wait for the current slot" rule

The most important invariant: **the fence wait happens for the slot you're about to use, not the one you just used.**

```
beginRender:
    wait on current slot's fence    <-- blocks until last use of this slot is done
    process current slot's destruction queue
    free current slot's staging buffers/textures, reset its descriptor allocator
                                    <-- safe now: GPU is finished with them
    acquire swapchain image
    reset current slot's fence
```

Resetting "previous slot" resources at frame start is a tempting-but-wrong shortcut — the previous slot's GPU work may not have finished yet. Always reset the *current* slot's resources after its fence signals.

### Submit and present

`submitCommands` and `endRender` are typically called together at frame end. The first uses `vkQueueSubmit2` with the current slot's fence and the image-acquire/render-complete semaphores. The second calls `vkQueuePresentKHR` on the present queue.

`endRender` also handles swapchain-out-of-date/suboptimal results by triggering a recreation. Recreation walks the swapchain destroy-create cycle and rebuilds per-image sync info.

---

## Resources and handles

Every backend resource lives in a `HandlePool` owned by `litl::vulkan::ResourceManager`. The manager exposes `createX`, `getX`, `destroyX` for each resource type. `getX` returns a pointer that's valid until that handle is destroyed or until the pool is reorganized — for short-lived lookups, the pointer is fine; never store it across frames.

Resources that have a logical "name" (textures, shader modules) get a secondary string-keyed map for name → handle lookups, used for hot reload and asset dedupe. See [Caches and hot reload](#caches-and-hot-reload).

### Deferred destruction

`Renderer::destroyTexture` and `Renderer::destroyBuffer(handle, /* immediate */ false)` don't destroy immediately — the resource may still be referenced by command buffers in flight. Instead they enqueue it on the **current** slot's `DestructionQueue`. (`destroyBuffer(handle, true)` destroys on the spot; only use it when the buffer is known to be unreferenced by the GPU.) That queue is processed the next time the slot comes around in `beginRender`, after its fence wait, so with N frames in flight a resource released on frame F is destroyed at the start of frame F+N. Hot reload uses the same queue for replaced pipelines and shader modules. Because there is one queue per slot, no per-item frame counting is needed.

A texture that occupies a texture-table slot releases it when destroyed; the slot is immediately rewritten to the fallback texture (see [Bindless textures](#bindless-textures)).

### Teardown

The `ResourceManager` destroys everything in `destroy()`, after a `vkDeviceWaitIdle`, in this order:

```
command buffers     →  vkFreeCommandBuffers
graphics pipelines  →  vkDestroyPipeline
pipeline layouts    →  vkDestroyPipelineLayout + DescriptorSetLayout   (PipelineLayoutCache)
buffers             →  vmaDestroyBuffer
samplers            →  vkDestroySampler
textures            →  vmaDestroyImage + vkDestroyImageView
shader modules      →  vkDestroyShaderModule
```

Pipelines reference layouts; layouts reference descriptor set layouts; textures reference image views. The texture table and sampler array are destroyed separately by the renderer.

---

## Pipelines

### Shaders are blobs, not stages

A `ShaderModuleHandle` corresponds to one SPIR-V binary, which may contain multiple entry points across multiple stages. A Slang file declaring both `vertexMain` and `fragmentMain` compiles to one `.spv`, which becomes one `VkShaderModule`. Pipelines reference the *(module, entry point name)* pair.

`reflectSPIRV` walks the bytecode and produces a `ShaderReflection` containing one `EntryPointReflection` per entry point, plus module-scoped specialization constants. Each entry point lists its resource bindings, push constant ranges, vertex inputs (vertex stage only), fragment outputs (fragment stage only), and compute local size (compute stage only).

### Pipeline layout from reflection

`createGraphicsPipeline` walks every populated shader slot in the `GraphicsPipelineDescriptor` and builds a `PipelineLayoutDescriptorCreateInfo`. The merger (`createPipelineLayoutDescriptor`) folds the per-stage reflections into a single `PipelineLayoutDescriptor`:

- Resource bindings collapse on `(set, binding)`, ORing stages, asserting type/array/size compatibility.
- Push constants collapse on exact `(offset, size)` matches, OR stages; partial overlaps reject.
- Spec constants live at module scope; the merger doesn't touch them.

The result feeds `getOrCreatePipelineLayout`, which hits `PipelineLayoutCache` to dedupe both descriptor set layouts and the combined pipeline layout. Identical reflections produce identical `VkDescriptorSetLayout` handles — that handle equality is what powers descriptor-set-layout compatibility checks later (see [Pipeline-switch dirty cascade](#pipeline-switch-dirty-cascade)).

### Descriptor set conventions

`DescriptorSetIndex` defines the per-set frequency tiers. Each higher index is also more volatile: disturbing set N forces every set above it to be rebound.

- **Set 0 — PerFrame**: in the Vulkan 1.4 path, this is the **global bindless texture table** (binding 0, runtime `Texture2D[]`) and the **predefined sampler array** (binding 1, `SamplerState[16]`). Classic per-frame data (camera, time, frame uniforms) is supplied through BDA instead. See [Bindless textures](#bindless-textures).
- **Set 1 — PerPass**: pass-specific shadow maps, environment data
- **Set 2 — PerMaterial**: material textures and parameters
- **Set 3 — PerObject**: per-draw indices (push descriptor)

Set 0 is bound once per render pass by `cmdBeginRender`. Sets 1–2 are bound via `vkCmdBindDescriptorSets` after pool allocation. Set 3 uses `vkCmdPushDescriptorSet`. Vulkan allows only one push set per pipeline layout — set 3 carries the flag, the others don't.

Every pipeline layout must have exactly `DescriptorSetMaxCount` (4) set layouts. The limit is deliberate: other potential backends (e.g. WebGPU) cap out at 4.

### Shader entry-point + stage validation

The merger validates each `(slot, entry-point)` pair: the entry point must exist in the module's reflection, and its reflected stage must match the slot it was placed in. A typo'd `entryPoint = "fragmentMain"` in the `.vertex` slot fails at merge time with `ErrorStageMismatch`, not with a confusing downstream error.

### Set 0 validation

When building a pipeline layout, the `PipelineLayoutCache` never creates a set layout for set 0 from reflection. It substitutes the texture table's global set layout. If the shader declares anything at set 0, it must match the global layout: binding 0 a runtime-sized `SampledImage` array, binding 1 a `Sampler` array of exactly 16. A mismatch is logged and pipeline-layout creation fails. A shader that declares nothing at set 0 passes through.

---

## Buffer subsystem

`BufferDescriptor` has four orthogonal axes:

- `BufferTypeFlag` — what the buffer is for (Vertex, Index, Uniform, Storage, TransferSrc, TransferDst, BufferDeviceAddress). Composable as flags.
- `BufferMemoryType` — preference for where memory lives (Auto, PreferGpu, PreferCpu).
- `BufferMemoryUsage` — how the CPU will touch it (GpuOnly, Staging, ReadBack, PersistentMap).
- `bytes` — size.

VMA picks the actual memory type based on these hints. Persistent-mapped buffers carry their CPU pointer in `allocationInfo.pMappedData`; if VMA can't get host-visible memory directly, it allocates a dedicated host-visible staging buffer that the renderer flushes implicitly.

### Persistent mapping vs staging

Two patterns:

**Persistent mapping** — for small, hot, per-frame data (UBO with view+projection). One buffer per frame-in-flight, indexed by `frameInFlightIndex`. `mapBuffer` returns the persistent CPU pointer; write directly, no driver round-trip per write. `unmapBuffer` does a flush (no-op on coherent memory).

**Staging upload** — for one-time GPU-only data (vertex/index buffers, textures). `cmdBufferUpload` and `cmdTextureUpload` copy CPU bytes into the current frame's staging arena, then record a `vkCmdCopyBuffer`/`vkCmdCopyBufferToImage` from staging to the GPU resource. `cmdBufferFlush` inserts the barrier that makes the data visible to subsequent reads. Wrap upload sequences in `cmdBeginBufferUpload` to get the flush implicitly on scope exit.

For first-time uploads at startup, use a one-shot transient command buffer instead — `createScopedCommandBuffer` plus the implicit flush — so the upload doesn't tangle with frame timing.

### Buffer Device Address

Buffers created with `BufferTypeFlagBits::BufferDeviceAddress` get a stable 64-bit GPU pointer, available via `getBufferDeviceAddress(buffer)` (no mapping required) or as `MappedBuffer::BufferDeviceAddress` after `mapBuffer`. Shaders dereference these pointers directly — no descriptor binding required. This is the recommended path for global storage buffers (transforms, materials, light lists, and per-frame data) — descriptor pressure drops, and indices become the natural per-draw parameter.

---

## Texture and sampler subsystem

### The image / view / sampler triplet

A "texture" you sample in a shader is three independent Vulkan objects:

- **`VkImage`** — the pixel storage, declared with a format, extent, mip count, array layer count, and usage flags.
- **`VkImageView`** — a typed view onto the image: which format to interpret as, which aspect (color/depth/stencil), which mip range and array range to expose.
- **`VkSampler`** — sampling state (filter, address mode, anisotropy, mip LOD bias). *Independent of any image* — one sampler is reused across many textures.

A `TextureResource` owns the image and view. Samplers live in their own pool, deduplicated by `SamplerCache` — identical `SamplerDescriptor` values produce the same `SamplerHandle`.

### Texture descriptors and uploads

Textures are described by `TextureResourceDescriptor` (`dimensions`, `width`/`height`/`depth`, `format`, `usage`, `mipLevels`, `arrayLayers`, `isCubeMap`, `residesInTextureTable`, an optional `name`, …) and referenced by `TextureResourceHandle`.

`cmdTextureUpload` comes in two forms:

- **`(cb, bytes, texture)`** — writes mip 0, layer 0.
- **`(cb, bytes, regions, texture)`** — writes any number of `TextureUploadRegion { sourceOffset, mipLevel, arrayLayer, width, height, depth }` from one source buffer. `buildTightlyPackedUploadRegions(descriptor, outRegions)` builds the region list for a tightly packed buffer holding every mip of every layer (level-major, then layer).

The renderer does not generate mipmaps; full mip chains are produced CPU-side (at import) and uploaded via regions.

### Layout transitions

Sampled textures go through three layouts during their lifecycle:

```
UNDEFINED  →  TRANSFER_DST_OPTIMAL     (preparing to upload)
TRANSFER_DST_OPTIMAL  →  SHADER_READ_ONLY_OPTIMAL    (after upload, before sampling)
```

`StagingTexture::copyIntoDestination` handles both transitions around a `vkCmdCopyBufferToImage2`, one copy per upload region. Once a texture is in `SHADER_READ_ONLY_OPTIMAL`, it stays there for its lifetime — no per-frame transitions.

### Separate textures and samplers in shaders

Shaders declare textures and samplers separately and combine them at the point of use. The common case is the global bindless arrays at set 0 (declared once in `assets/shaders/litl/core.slang`):

```hlsl
BindPerFrame(0) Texture2D<float4> g_Textures[];
BindPerFrame(1) SamplerState      g_Samplers[16];

// g_Textures[NonUniformResourceIndex(textureIndex)].Sample(g_Samplers[NonUniformResourceIndex(samplerIndex)], uv)
```

Explicitly bound per-pass/per-material textures follow the same separated pattern (`Texture2D` + `SamplerState` at their own bindings). The combined-sampler form (`Sampler2D`) also works and reflects as a combined descriptor type, but the separated form is the forward path.

---

## Bindless textures

### The texture table

`TextureTable` is a single, global, update-after-bind descriptor set: one runtime-sized array of sampled images at set 0, binding 0. Its capacity is the smaller of `RendererConfiguration::globalTexturePoolCapacity` (default 16384) and the device's update-after-bind sampled-image limit.

- A texture opts in with `TextureResourceDescriptor::residesInTextureTable = true` (typical for asset-loaded textures; runtime render targets usually don't). On creation the `ResourceManager` **acquires** a slot (free list first, then a bump head) and writes the image view into it.
- `Renderer::getTextureTableIndex(texture)` returns the slot, or the `uint32_t` null index if the texture isn't in the table. That index is what goes into material data.
- On destruction the slot is **released** and immediately rewritten to point at the fallback (slot 0), so a stale index samples pink instead of a dead descriptor.
- The descriptors use `PARTIALLY_BOUND | UPDATE_AFTER_BIND`, so slots can be written while the set is bound and unused slots need not be valid.

### Reserved slots

`TextureTableReservedIndices` reserves the first four slots for 1×1 defaults that the engine's `RenderManager` creates at startup using `textureTableIndexOverride`: `Pink` (0, the missing-texture fallback), `White` (1), `Black` (2), and `Normal` (3, a flat tangent-space normal). `getReservedTextureTableIndex(name)` maps a name to one of these. `textureTableIndexOverride` is otherwise discouraged.

### The sampler array

`SamplerArray` holds the 16 `SamplerPredefines` (`LinearRepeat`, `LinearClamp`, `LinearRepeatAniso`, `LinearClampAniso`, `NearestRepeat`, `NearestClamp`, …, with the remainder reserved), created through `SamplerCache` from `SamplerPredefinedDescriptors` and written to set 0, binding 1.

### Packed texture + sampler index

`litl-renderer/utility.hpp` packs a texture slot and sampler index into one `uint32_t` — slot in the low 24 bits (≈16.7M textures), sampler in the high 8 bits — via `packTextureSlotSamplerIndex`, `extractTextureSlot`, and `extractSamplerIndex`. Shader-side, `sampleGlobalTexture(packed, uv)` in `core.slang` unpacks and samples.

---

## Descriptor binding

### Push descriptors for transient, allocated for stable

Push descriptors (`vkCmdPushDescriptorSet`) carry the per-draw cost of binding directly in the command buffer. They're cheap when the binding count is small (one or two) and they change every draw. Set 3 (PerObject) uses push.

For stable bindings (PerFrame, PerPass, PerMaterial), the cost of writing a descriptor set once and binding it many times is lower. The `DescriptorSetAllocator` (one instance per frame-in-flight slot) manages pool churn — at frame start, the slot's allocator does `vkResetDescriptorPool` (safe because the slot's fence guarantees its previous work is done), and subsequent allocations come from a fresh pool.

### The change tracker

`DescriptorSetChangeTracker` lives on each `CommandBufferResource`. Its job is to defer the actual Vulkan binding work — `vkCmdBindDescriptorSets`, `vkCmdPushDescriptorSet`, `vkUpdateDescriptorSets`, descriptor set allocations — to the moment of draw.

Three mental hooks:

1. **`addChange(set, binding, type, info)`** — called by `cmdBindBuffer` / `cmdBindTexture` / `cmdBindSampler` when the user binds a named resource. The tracker keeps a *current state* per set: if a binding number already exists in that set, replace it; otherwise append. Mark the set dirty.

2. **`onPipelineLayoutChange(prev, curr)`** — called by `cmdBindGraphicsPipeline`. Compares the two pipelines' per-set layouts via handle equality. Finds the first set where they diverge, dirties everything from there up. Pending writes survive — they describe *what should be in the set*, not which pipeline they were authored against.

3. **`flushChanges(...)`** — called by `cmdDraw`. For each dirty set: if it's the push set, build writes and `vkCmdPushDescriptorSet`; otherwise allocate, update, bind. Pending state is not cleared — same writes can be re-emitted on the next dirty-set cycle without re-binding source resources.

### Pipeline-switch dirty cascade

Vulkan's compatibility rule: two pipeline layouts are "compatible for set N" if all sets `[0..N]` have identical descriptor set layouts. A mismatch at set K invalidates K and every higher set.

The cascade matters because reflection-derived layouts dedupe via `PipelineLayoutCache`. Set 0 is always the same global texture-table layout, so a pipeline switch never disturbs it. Two materials whose PerPass layouts both contain the same shadow-map binding produce the same `VkDescriptorSetLayout` handle — so a material switch doesn't disturb set 1 either, and its existing binding survives across draws. PerMaterial typically *does* differ, so set 2 dirties. PerObject is push and binds per-draw regardless.

### Frame-start reset

`resetTransient` on the descriptor allocator is paired with `tracker.reset()` on the command buffer. Both happen at frame start, after the fence wait. The allocator wipes all sets from the pool; the tracker wipes pending state and dirty bits. Without both, the tracker would think sets are bound that no longer exist.

---

## Caches and hot reload

### Pipeline layout cache

`PipelineLayoutCache` is a two-level cache:

- **Inner**: `DescriptorSetLayoutCacheKey` → `VkDescriptorSetLayout`. The key is the reflected `DescriptorSetLayoutDesc` *plus* `DescriptorSetLayoutOptions` (the per-type runtime-array capacities and whether it's the push set), since both change the created object.
- **Outer**: `(set layout handles[], push constant ranges[])` → `VkPipelineLayout`.

Bindings with `arraySize == 0` are runtime arrays: their descriptor count comes from the capacities, they get `PARTIALLY_BOUND | UPDATE_AFTER_BIND`, and the layout gets `UPDATE_AFTER_BIND_POOL`. A push set may not contain runtime arrays (asserted), and runtime arrays of acceleration structures aren't supported.

Both maps grow during a run and clear only at `destroy()`. Hashing is sensitive to struct padding — `static_assert`s pin the POD layouts that are hashed bytewise, and the cache key hashes its padded options struct field by field.

### Sampler cache

`SamplerCache` deduplicates `VkSampler` objects by a hash over `SamplerDescriptor` — `createSampler` with an existing descriptor returns the existing handle. In practice most sampling goes through the 16 predefined samplers in the global `SamplerArray`; custom samplers are for explicitly bound textures.

### Shader hot reload

`ShaderModuleReferenceMap` tracks which pipelines (graphics or compute) reference each shader module. When a `.slang` recompiles and the engine calls `reloadShaderModule(descriptor)`:

1. Hash the new SPIR-V. If unchanged, destroy the new module and return — common case.
2. Replace the old module's contents *in place* (new `VkShaderModule`, new reflection, new hash). The `ShaderModuleHandle` and `ShaderModuleResource*` stay valid — no outside code needs to know.
3. Look up all pipelines referencing this shader. For each, rebuild the pipeline resource into a staging slot using its preserved `GraphicsPipelineDescriptor`. On success, swap `vkPipeline` and enqueue the old one on the current slot's destruction queue.
4. Enqueue the old `VkShaderModule` for deferred destruction.

The "in-place update" trick preserves handle stability — user code holding `ShaderModuleHandle` or `GraphicsPipelineHandle` doesn't need to know anything changed. The user's draw code, written against the handles, just keeps working with the new SPIR-V.

---

## Conventions and invariants

### Coordinate system

- **Left-handed world space**: +X right, +Y up, +Z forward.
- **Y-up convention**: world up is `(0, 1, 0)`.
- **Reversed-Z depth**: near plane → 1, far plane → 0. Pairs with `CompareOp::Greater`. Better precision distribution at distance.
- **Winding order**: default is clockwise (`RasterizationState::frontFace = FrontFace::Clockwise`)
- **Vulkan Y-down NDC compensation**: `mat4::perspective` applies a `proj[1][1] *= -1` after `glm::perspectiveLH` so world-Y-up vertices appear right-side-up on screen.

### Descriptor set frequency tiers

`DescriptorSetIndex` is a convention enforced by reflection. Shaders should place bindings at the right set:

- Set 0: reserved for the global texture table + sampler array. Per-frame data goes through BDA.
- Set 1: anything that's constant for an entire pass.
- Set 2: anything that's constant for a material instance.
- Set 3: anything that changes per draw (object indices, etc.). Push descriptor — keep tiny.

Putting per-pass data in set 2 works mechanically but defeats the cache-hit pattern in the pipeline-switch cascade. Putting anything other than the global arrays in set 0 fails pipeline-layout creation.

### Frames-in-flight resource duplication

Anything the CPU writes every frame needs `framesInFlight` copies, indexed by `frameInFlightIndex`. UBOs, descriptor sets, persistent-mapped buffers. The reason: the CPU writes frame N+1 while the GPU reads frame N — they can't share the same memory.

The exception is read-only data (vertex buffers, textures) — one copy on the GPU, read by all frames.

---

## Common patterns

### Loading a shader

```cpp
std::ifstream file(path, std::ios::ate | std::ios::binary);
auto fileSizeBytes = static_cast<size_t>(file.tellg());
AlignedByteBuffer<4> byteBuffer{ fileSizeBytes };  // SPIR-V requires 4-byte alignment
file.seekg(0);
file.read(byteBuffer.as<char>().data(), byteBuffer.size());

ShaderModuleHandle handle = renderer->createShaderModule(ShaderModuleDescriptor{
    .resource = path,
    .bytes    = byteBuffer.as<std::byte>(),
});
```

### Uploading vertex data once at startup

```cpp
auto scopedCb = renderer->createScopedCommandBuffer();  // one-shot, blocking submit on destruction

BufferDescriptor desc{
    .type  = BufferTypeFlagBits::VertexBuffer | BufferTypeFlagBits::TransferDest,
    .bytes = vertices.size() * sizeof(Vertex),
};
BufferHandle vertexBuffer = renderer->createBuffer(desc);
renderer->cmdBufferUpload(scopedCb.get(), as_byte_span(vertices), vertexBuffer);

// scopedCb destructor flushes, submits, waits, frees the command buffer.
```

### Per-frame UBO with persistent mapping

```cpp
// At init: one buffer per frame in flight.
for (uint32_t i = 0; i < framesInFlight; ++i) {
    sample.uboBuffers.push_back(renderer->createBuffer(BufferDescriptor{
        .type        = BufferTypeFlagBits::UniformBuffer,
        .memoryUsage = BufferMemoryUsage::PersistentMap,
        .bytes       = sizeof(MyUbo),
    }));
}

// At frame start: write the current slot's copy.
auto buf = sample.uboBuffers[frameData.frameInFlightIndex];
MappedBuffer mapped{};
renderer->mapBuffer(buf, mapped);
std::memcpy(mapped.mappedPtr, &myUbo, sizeof(MyUbo));
renderer->unmapBuffer(buf);  // flushes if not coherent
```

### Binding per-frame data via BDA

```cpp
// Create a buffer with shader device address.
BufferHandle bdaBuffer = renderer->createBuffer(BufferDescriptor{
    .type        = BufferTypeFlagBits::BufferDeviceAddress,
    .memoryUsage = BufferMemoryUsage::PersistentMap,
    .bytes       = sizeof(MyData),
});

// At frame time: write data, fetch address.
MappedBuffer mapped{};
renderer->mapBuffer(bdaBuffer, mapped);
std::memcpy(mapped.mappedPtr, &myData, sizeof(MyData));
pushConstants.dataAddress = mapped.BufferDeviceAddress;   // or renderer->getBufferDeviceAddress(bdaBuffer).value()

// Push the address through push constants. The shader dereferences:
//   struct PushConstants { MyData *data; };
//   _pc.data->field
renderer->cmdPushConstants(cb, ShaderStage::Fragment, generic_as_byte_span(&pushConstants, sizeof(PushConstants)));
```

### A draw call with allocated + push descriptors

```cpp
renderer->cmdBindGraphicsPipeline(cb, materialPipeline);
renderer->cmdBindBuffer          (cb, passUbo,      "_pass"_sid,     true);  // PerPass set
renderer->cmdBindBuffer          (cb, materialUbo,  "_material"_sid, true);  // PerMaterial set
renderer->cmdBindTexture         (cb, detailTex,    "_detail"_sid,   true);  // explicitly bound texture
renderer->cmdBindSampler         (cb, detailSampler,"_detailSampler"_sid, true);
renderer->cmdBindVertexBuffer    (cb, vertexBuffer);
renderer->cmdBindIndexBuffer     (cb, indexBuffer);
renderer->cmdDrawIndexed         (cb, indexCount, 1, 0, 0, 0);
// On cmdDrawIndexed, the tracker flushes all dirty sets.
// Textures in the global table need no binding at all: pass their packed slot/sampler index in material data.
```

---

## What's not yet here

Inventory of things the renderer doesn't do yet, for context on the gaps:

- **GPU mipmap generation** — multi-mip textures are supported, but mips must be generated CPU-side (as the importer does) and uploaded via regions; there's no blit-based generation.
- **Cube maps** — descriptor and image-creation paths support them, but nothing in the engine creates one yet.
- **Compute pipelines** — descriptors and command ops exist, but `createComputePipeline` is a `todo` and returns an invalid handle. The global texture table is also only bound for graphics.
- **Indirect draw** — `vkCmdDrawIndirect` not exposed.
- **Multi-pass / rendergraph** — single-pass only; no automatic barrier scheduling.
- **MSAA / multisample resolve** — `MultisampleState` exists but the swapchain is single-sample.
- **Texture hot reload** — `onTextureReload` is stubbed.
- **PSO content cache** — `vkCmdCreateGraphicsPipelines` always uses the on-disk `VkPipelineCache`, but no descriptor-content dedup.
- **Multi-queue / async transfer** — single graphics queue handles everything including uploads.
- **Bindless beyond textures** — sampled images are bindless via the texture table and buffers are reachable via BDA, but there are no bindless storage-image or buffer descriptor arrays.

Each of these is a deliberate "phase N+1" deferral. The current shape doesn't lock any of them out.

---

## Useful files to read

When the document is no longer enough, these files are the load-bearing ones:

| File | What lives here |
|------|-----------------|
| `litl/renderer/include/litl-renderer/renderer.hpp` | `RendererOps` table + `Renderer` wrapper |
| `litl/renderer-vulkan/include/litl-renderer-vulkan/rendererContext.hpp` | Concrete Vulkan `RendererContext` |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/renderer.cpp` | Device/instance/swapchain creation |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/resourceManager.cpp` | `createBuffer`, `createTexture`, `createGraphicsPipeline` |
| `litl/renderer/include/litl-renderer/reflection.hpp` | Reflection structures, merger error enum |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/resources/pipelineLayoutDescriptor.cpp` | The reflection merger |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/resources/cache/pipelineLayoutCache.cpp` | Layout dedup |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/resources/utility/descriptorSetChangeTracker.cpp` | The deferred binding tracker |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/resources/utility/textureTable.cpp` | Global bindless texture table: slots, fallback, set 0 layout |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/resources/utility/samplerArray.cpp` | The 16 predefined samplers |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/resources/utility/destructionQueue.cpp` | Per-frame-in-flight deferred destruction |
| `litl/renderer-vulkan/src/litl-renderer-vulkan/requiredFeatures.cpp` | Required Vulkan 1.4 device features |
| `litl/renderer/include/litl-renderer/resources/texture.hpp` | `TextureResourceDescriptor`, upload regions, reserved table indices |
| `litl/renderer/include/litl-renderer/resources/sampler.hpp` | `SamplerDescriptor`, `SamplerPredefines` |
| `assets/shaders/litl/core.slang` | Shader-side set 0 declarations and `sampleGlobalTexture` |
| `samples/renderer/src/main.cpp` | Working example exercising every subsystem |

The sample is the most accurate documentation — it's the one path through the API that's known to draw a textured triangle without validation errors.
