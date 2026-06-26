# LITL Renderer — Design Reference

A high-level overview of `litl-renderer` and `litl-renderer-vulkan`. Written for someone (probably future-you) who needs to understand the design well enough to use the renderer or extend it.

This is not API documentation — header comments handle that. This is the *why* behind the shape.

---

## Overview

The renderer is split into two layers:

- **`litl-renderer`** — a backend-agnostic API. Declares all the public types, descriptors, the `Renderer` wrapper class, and a function-pointer table (`RendererOps`). Knows nothing about Vulkan, D3D12, Metal, or anything else. User code includes only these headers.
- **`litl-renderer-vulkan`** — the Vulkan implementation. Defines concrete versions of every opaque type and fills in every function in the ops table. The user instantiates a renderer via `createVulkanRenderer(...)` and from then on talks only to the abstract API.

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
        renderer->cmdBeginRender(cb, BeginRenderCommand{
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

Resources cross the boundary as typed integer handles, not pointers. `BufferHandle`, `TextureHandle`, `ShaderModuleHandle`, etc. are thin wrappers around a generation+index pair (`Handle<Tag>`). Storage lives backend-side in `HandlePool<Resource, Tag>`s; user code never sees a `VkBuffer`.

Handles survive backend resource churn (hot reload, recreation) because the *handle* is stable while the *underlying resource* gets rebuilt in place. See [Hot reload](#hot-reload).

---

## Frame loop and synchronization

### Frames in flight

The renderer keeps `framesInFlight` slots (typically 2). Each slot has its own:

- **Command buffer** — recorded into during the frame, submitted at frame end.
- **Render fence** — signals when the GPU is done with this slot's work.
- **Semaphores** — image-acquire and render-complete for ordering swap.
- **Staging arenas** (buffer + texture) — per-frame transfer scratch space.
- **Descriptor set allocator** — pool used for transient descriptor allocations this frame.

The slot index cycles every frame: `slot = frameCount % framesInFlight`. With two slots, by the time we return to slot 0 again, the GPU has had two frames' worth of time to finish slot 0's previous work.

### The "wait for the current slot" rule

The most important invariant: **the fence wait happens for the slot you're about to use, not the one you just used.**

```
beginRender:
    wait on current slot's fence    <-- blocks until last use of this slot is done
    reset current slot's resources  <-- safe now: GPU is finished with them
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

The `ResourceManager` also destroys everything in `destroy()`. Order matters:

```
graphics pipelines  →  vkDestroyPipeline
command buffers     →  vkFreeCommandBuffers
pipeline layouts    →  vkDestroyPipelineLayout + DescriptorSetLayout
buffers             →  vmaDestroyBuffer
samplers            →  vkDestroySampler
textures            →  vmaDestroyImage + vkDestroyImageView
shader modules      →  vkDestroyShaderModule
```

Pipelines reference layouts; layouts reference descriptor set layouts; textures reference image views. Reverse-creation order destroys.

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

`DescriptorSetIndex` defines the per-set frequency tiers:

- **Set 0 — PerFrame**: camera/view/projection, time, frame uniforms
- **Set 1 — PerPass**: pass-specific shadow maps, environment data
- **Set 2 — PerMaterial**: material textures and parameters
- **Set 3 — PerObject**: per-draw indices (push descriptor)

Sets 0–2 are bound via `vkCmdBindDescriptorSets` after pool allocation. Set 3 uses `vkCmdPushDescriptorSet`. Vulkan allows only one push set per pipeline layout — set 3 carries the flag, the others don't.

### Shader entry-point + stage validation

The merger validates each `(slot, entry-point)` pair: the entry point must exist in the module's reflection, and its reflected stage must match the slot it was placed in. A typo'd `entryPoint = "fragmentMain"` in the `.vertex` slot fails at merge time with `ErrorStageMismatch`, not with a confusing downstream error.

---

## Buffer subsystem

`BufferDescriptor` has four orthogonal axes:

- `BufferTypeFlag` — what the buffer is for (Vertex, Index, Uniform, Storage, TransferSrc, TransferDst, ShaderDeviceAddress). Composable as flags.
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

Buffers created with `BufferTypeFlagBits::ShaderDeviceAddress` get a stable 64-bit GPU pointer (`bdaAddress`), accessible via `mapBuffer().shaderDeviceAddress`. Shaders dereference these pointers directly — no descriptor binding required. This is the recommended path for global storage buffers (transforms, materials, light lists) — descriptor pressure drops, and indices become the natural per-draw parameter.

---

## Texture and sampler subsystem

### The image / view / sampler triplet

A "texture" you sample in a shader is three independent Vulkan objects:

- **`VkImage`** — the pixel storage, declared with a format, extent, mip count, array layer count, and usage flags.
- **`VkImageView`** — a typed view onto the image: which format to interpret as, which aspect (color/depth/stencil), which mip range and array range to expose.
- **`VkSampler`** — sampling state (filter, address mode, anisotropy, mip LOD bias). *Independent of any image* — one sampler is reused across many textures.

A `TextureResource` owns the image and view. Samplers live in their own pool, deduplicated by `SamplerCache` — identical `SamplerDescriptor` values produce the same `SamplerHandle`.

### Layout transitions

Sampled textures go through three layouts during their lifecycle:

```
UNDEFINED  →  TRANSFER_DST_OPTIMAL     (preparing to upload)
TRANSFER_DST_OPTIMAL  →  SHADER_READ_ONLY_OPTIMAL    (after upload, before sampling)
```

`StagingTexture::copyIntoDestination` handles both transitions around a `vkCmdCopyBufferToImage2`. Once a texture is in `SHADER_READ_ONLY_OPTIMAL`, it stays there for its lifetime — no per-frame transitions.

### Separate textures and samplers in shaders

Shaders declare textures and samplers separately:

```hlsl
[vk::binding(1, 0)] Texture2D<float4> _texture;
[vk::binding(2, 0)] SamplerState      _texture_sampler;
```

…and combine them at usage with `_texture.Sample(_texture_sampler, uv)`. The combined-sampler form (`Sampler2D`) also works and reflects as a combined descriptor type, but the separated form is the forward path — bindless texture arrays naturally use separate sampler + image arrays.

---

## Descriptor binding

### Push descriptors for transient, allocated for stable

Push descriptors (`vkCmdPushDescriptorSet`) carry the per-draw cost of binding directly in the command buffer. They're cheap when the binding count is small (one or two) and they change every draw. Set 3 (PerObject) uses push.

For stable bindings (PerFrame, PerPass, PerMaterial), the cost of writing a descriptor set once and binding it many times is lower. The `DescriptorSetAllocator` (one instance per frame-in-flight slot) manages pool churn — at frame start, the slot's allocator does `vkResetDescriptorPool` (safe because the slot's fence guarantees its previous work is done), and subsequent allocations come from a fresh pool.

### The change tracker

`DescriptorSetChangeTracker` lives on each `CommandBufferResource`. Its job is to defer the actual Vulkan binding work — `vkCmdBindDescriptorSets`, `vkCmdPushDescriptorSet`, `vkUpdateDescriptorSets`, descriptor set allocations — to the moment of draw.

Three mental hooks:

1. **`addChange(set, binding, type, info)`** — called by `cmdBindGraphicsBuffer` / `cmdBindTexture` etc. when the user binds a named resource. The tracker keeps a *current state* per set: if a binding number already exists in that set, replace it; otherwise append. Mark the set dirty.

2. **`onPipelineLayoutChange(prev, curr)`** — called by `cmdBindGraphicsPipeline`. Compares the two pipelines' per-set layouts via handle equality. Finds the first set where they diverge, dirties everything from there up. Pending writes survive — they describe *what should be in the set*, not which pipeline they were authored against.

3. **`flushChanges(...)`** — called by `cmdDraw`. For each dirty set: if it's the push set, build writes and `vkCmdPushDescriptorSet`; otherwise allocate, update, bind. Pending state is not cleared — same writes can be re-emitted on the next dirty-set cycle without re-binding source resources.

### Pipeline-switch dirty cascade

Vulkan's compatibility rule: two pipeline layouts are "compatible for set N" if all sets `[0..N]` have identical descriptor set layouts. A mismatch at set K invalidates K and every higher set.

The cascade matters because reflection-derived layouts dedupe via `PipelineLayoutCache`. Two materials whose PerFrame layouts both contain `_view` + `_projection` produce the same `VkDescriptorSetLayout` handle — so a material switch doesn't disturb set 0, and that set's existing binding survives across draws. PerMaterial typically *does* differ, so set 2 dirties. PerObject is push and binds per-draw regardless.

### Frame-start reset

`resetTransient` on the descriptor allocator is paired with `tracker.reset()` on the command buffer. Both happen at frame start, after the fence wait. The allocator wipes all sets from the pool; the tracker wipes pending state and dirty bits. Without both, the tracker would think sets are bound that no longer exist.

---

## Caches and hot reload

### Pipeline layout cache

`PipelineLayoutCache` is a two-level cache:

- **Inner**: `DescriptorSetLayoutDesc` → `VkDescriptorSetLayout`, keyed on bytewise hash of the descriptor.
- **Outer**: `(set layout handles[], push constant ranges[])` → `VkPipelineLayout`.

Both maps grow during a run and clear only at `destroy()`. The inner map's hashing is sensitive to struct padding — a `static_assert` in the header pins the struct layout so silent hash mismatches don't sneak in if someone changes an underlying enum's underlying type.

### Sampler cache

`SamplerCache` deduplicates `VkSampler` objects by a hash over `SamplerDescriptor`. Most engines need ~5 samplers total (LinearWrap, LinearClamp, PointWrap, PointClamp, anisotropic) and the cache enforces that at the API level — `createSampler` with an existing descriptor returns the existing handle.

### Shader hot reload

`ShaderModuleReferenceMap` tracks which pipelines (graphics or compute) reference each shader module. When a `.slang` recompiles and the engine calls `reloadShaderModule(descriptor)`:

1. Hash the new SPIR-V. If unchanged, destroy the new module and return — common case.
2. Replace the old module's contents *in place* (new `VkShaderModule`, new reflection, new hash). The `ShaderModuleHandle` and `ShaderModuleResource*` stay valid — no outside code needs to know.
3. Look up all pipelines referencing this shader. For each, rebuild the pipeline resource into a staging slot using its preserved `GraphicsPipelineDescriptor`. On success, swap `vkPipeline` and destroy the old.
4. Destroy the old `VkShaderModule`.

The "in-place update" trick preserves handle stability — user code holding `ShaderModuleHandle` or `GraphicsPipelineHandle` doesn't need to know anything changed. The user's draw code, written against the handles, just keeps working with the new SPIR-V.

Caveat: this currently uses `vkDeviceWaitIdle` (or should — check `onShaderModuleReload`) to ensure the GPU isn't using the old objects. For a production engine, defer the old object's destruction by `framesInFlight` instead.

---

## Conventions and invariants

### Coordinate system

- **Left-handed world space**: +X right, +Y up, +Z forward.
- **Y-up convention**: world up is `(0, 1, 0)`.
- **Reversed-Z depth**: near plane → 1, far plane → 0. Pairs with `CompareOp::Greater`. Better precision distribution at distance.
- **Vulkan Y-down NDC compensation**: `mat4::perspective` applies a `proj[1][1] *= -1` after `glm::perspectiveLH` so world-Y-up vertices appear right-side-up on screen. Side effect: triangle winding flips relative to GL conventions — the renderer's default `FrontFace::CounterClockwise` accounts for this.

### Descriptor set frequency tiers

`DescriptorSetIndex` is a soft convention enforced by reflection. Shaders should place bindings at the right set:

- Set 0: anything that's constant for an entire frame.
- Set 1: anything that's constant for an entire pass.
- Set 2: anything that's constant for a material instance.
- Set 3: anything that changes per draw (object indices, etc.). Push descriptor — keep tiny.

Putting per-frame data in set 2 works mechanically but defeats the cache-hit pattern in the pipeline-switch cascade.

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
auto mapped = renderer->mapBuffer(buf);
std::memcpy(mapped.mappedPtr, &myUbo, sizeof(MyUbo));
renderer->unmapBuffer(buf);  // flushes if not coherent
```

### Binding per-frame data via BDA

```cpp
// Create a buffer with shader device address.
BufferHandle bdaBuffer = renderer->createBuffer(BufferDescriptor{
    .type        = BufferTypeFlagBits::ShaderDeviceAddress,
    .memoryUsage = BufferMemoryUsage::PersistentMap,
    .bytes       = sizeof(MyData),
});

// At frame time: write data, fetch address.
auto mapped = renderer->mapBuffer(bdaBuffer);
std::memcpy(mapped.mappedPtr, &myData, sizeof(MyData));
pushConstants.dataAddress = mapped.shaderDeviceAddress;

// Push the address through push constants. The shader dereferences:
//   struct PushConstants { MyData *data; };
//   _pc.data->field
renderer->cmdPushConstants(cb, ShaderStage::Fragment,
    generic_as_byte_span(&pushConstants, sizeof(PushConstants)));
```

### A draw call with allocated + push descriptors

```cpp
renderer->cmdBindGraphicsPipeline(cb, materialPipeline);
renderer->cmdBindGraphicsBuffer (cb, perFrameUbo,  "_view"_sid);     // PerFrame set
renderer->cmdBindGraphicsBuffer (cb, materialUbo,  "_material"_sid); // PerMaterial set
renderer->cmdBindTexture        (cb, albedoTex, "_albedo"_sid,
                                     linearSampler, "_albedoSampler"_sid);
renderer->cmdBindVertexBuffer   (cb, vertexBuffer);
renderer->cmdBindIndexBuffer    (cb, indexBuffer);
renderer->cmdDrawIndexed        (cb, indexCount, 1, 0, 0, 0);
// On cmdDrawIndexed, the tracker flushes all dirty sets.
```

---

## What's not yet here

Inventory of things the renderer doesn't do yet, for context on the gaps:

- **Mipmap generation** — single-mip textures only; layout transitions assume `levelCount = 1`.
- **Cube maps** — descriptor and image-creation paths support them, but the image-view layer count is wrong for cube views (uses post-divided `layerCount`).
- **Compute pipelines** — descriptors and command ops exist as stubs; no implementation.
- **Indirect draw** — `vkCmdDrawIndirect` not exposed.
- **Multi-pass / rendergraph** — single-pass only; no automatic barrier scheduling.
- **MSAA / multisample resolve** — `MultisampleState` exists but the swapchain is single-sample.
- **Texture hot reload** — `onTextureReload` is stubbed.
- **PSO content cache** — `vkCmdCreateGraphicsPipelines` always uses the on-disk `VkPipelineCache`, but no descriptor-content dedup.
- **Multi-queue / async transfer** — single graphics queue handles everything including uploads.
- **Bindless** — the descriptor model is per-binding; no `RUNTIME_DESCRIPTOR_ARRAY` support yet.

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
| `samples/renderer/src/main.cpp` | Working example exercising every subsystem |

The sample is the most accurate documentation — it's the one path through the API that's known to draw a textured triangle without validation errors.
