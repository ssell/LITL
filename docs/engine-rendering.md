# LITL Engine Rendering — Design Reference

A high-level overview of how `litl-engine` turns a scene into a frame: views, render passes, lighting, HDR output, and directional cascaded shadow maps.

This document covers the **engine** layer (`litl/engine/.../render`, `litl/engine/.../lights`, and the engine shaders in `assets/litl/shaders`). For the backend-agnostic renderer API and its Vulkan implementation (handles, pipelines, the bindless texture table, barriers), see [`renderer.md`](./renderer.md). For cameras, world matrices, and spatial queries, see [`scene.md`](./scene.md).

## Overview

Rendering is split across a handful of services and one ECS system:

- **`LightManager`** — collects the lights that are active this frame and uploads them to the GPU.
- **`ShadowManager`** — decides which shadow maps exist this frame, allocates layers for them in a shared pool texture, fits a light-space projection for each, and uploads the per-camera data the shaders sample with.
- **`CullingSystem`** (`PreRender`) — frustum-culls renderables once for every view: each camera and each shadow map.
- **`RenderManager`** — turns the culled results into `RenderView`s, uploads the per-view data, and runs the render passes in a fixed order.
- **`RenderPass`** subclasses — `ShadowPass`, `DepthOnlyPass`, `ForwardOpaquePass`, `ForwardTransparentPass`, and `BlitPass`. Each picks the views it cares about and draws them.

The governing idea: **everything that varies per frame is decided on the CPU before `Render` starts, and reaches the GPU as flat structs behind buffer device addresses.** Shaders get one push-constant block of addresses and never see an engine type.

---

## The frame, end to end

```
LateUpdate
    DirectionalLightSystem ──► LightManager::markActive   (deferred command, per job)

PreRender onPreGroup  (EngineCallbacks, single-threaded, in this order)
    1. LightManager::onPreRender     drain light commands, upload light buffers
    2. SceneManager::onPreRender     world matrices, world bounds, cameras finalized
    3. ShadowManager::onPreRender    allocate layers, fit cascades, build ShadowViews
    4. MaterialManager::onPreRender
    5. user pre-group callbacks

PreRender systems
    ... material systems ...
    CullingSystem                    one CullView per camera + one per ShadowView

Render
    RenderManager::onRender
        sort      → per-view entity lists (opaque by material/mesh, transparent back-to-front)
        upload    → PerFrameData, world matrices, PerPassData[], InstanceData[]
        passes    → Shadow → DepthOnly → ForwardOpaque → ForwardTransparent → Blit
```

The order inside the `PreRender` pre-group matters:

- **Shadows need finalized cameras.** `ShadowManager` fits cascades to each camera's frustum, so it must run after `SceneManager::onPreRender`.
- **Culling needs shadow views.** `CullingSystem::prepare` reads `ShadowManager::getShadowViews()`, so shadow views must exist before the `PreRender` systems run.
- **The light buffer is uploaded before shadows are built.** So nothing about shadows is written back into the light data. The link from a camera's shadow data back to its light goes the other way (`ShadowRenderViewData::lightIndex`).

---

## Views

A *view* is one thing being rendered from one point of view. There are three kinds, and they flow through three structs.

### CullView → PreparedView → RenderView

| Stage | Struct | Built by | Holds |
|-------|--------|----------|-------|
| Culling | `CullView` | `CullingSystem::prepare` | frustum, camera (or null), `sourceIndex`, `CullViewType`, `wantsTransparent` |
| Layout | `PreparedView` (internal) | `RenderManager::gatherPreparedViews` | `RenderViewType`, entity spans, `viewIndex`, instance-buffer offsets, priority |
| Drawing | `RenderView` | `RenderManager::writeViewData` | `ResolvedView`, render target, sorted entity spans, per-view buffer addresses, shadow bias |

`sourceIndex` is how a view finds its origin again. For a camera it is the camera's slot in `SceneView::getCameras()`. For a shadow view it is the index into `ShadowManager::getShadowViews()`.

### View types

| `RenderViewType` | Comes from | Drawn by | Target |
|------------------|------------|----------|--------|
| `Camera` | a camera with color enabled | ForwardOpaque, ForwardTransparent, Blit | the camera's `RenderTexture` |
| `DepthOnly` | a camera whose color is disabled | DepthOnly, Blit (as a depth visualization) | the camera's `RenderTexture` depth |
| `Shadow` | a `ShadowView` | Shadow | one layer of a shadow pool texture |

A camera is culled only if it is active and has a render texture. Camera views with nothing visible are skipped. **Shadow views are never skipped**, even with no casters, because the pass is what clears the layer; a skipped layer would keep stale depth from an earlier frame.

### ResolvedView

`RenderView::view` is a `ResolvedView`: the view, projection, and view-projection matrices, eye position, viewport, frustum, color/depth enables, clear color, and near/far. It is built by `ResolvedView::fromCamera` or `ResolvedView::fromShadowView`, so passes never need to know whether a view came from a `Camera`. This is the only reason a shadow view can be drawn by the same `RenderPass` machinery as a camera.

A shadow view has no `RenderTexture`. Instead it sets `RenderView::depthTargetOverrideHandle` (the pool texture) and `depthTargetOverrideLayer` (its layer), and `RenderPass::resolveRenderTarget` uses those in place of the render texture's depth.

### Ordering

`writeViewData` stable-sorts the `RenderView`s by camera priority, lowest first. Shadow views have priority 0. Each pass then walks the whole list and filters by type, so pass order (not view order) decides what renders first: every shadow map is finished before any camera is drawn.

---

## Per-frame GPU data

All engine shader inputs are reached through buffer device addresses in one push-constant block, `RenderPushConstants`:

| Field | Points to | Granularity |
|-------|-----------|-------------|
| `perFrameDataAddr` | `RenderPerFrameData` (frame, frame index, elapsed time, delta time) | per frame |
| `perPassDataAddr` | one `RenderPerPassData` | per view |
| `instanceDataAddr` | the view's `RenderInstanceData[]` (opaque or transparent run) | per view, per pass |
| `worldMatricesAddr` | every tracked entity's world matrix, indexed by GPU index | per frame |
| `materialPropertiesAddr` | the bound material's property array | per draw item |
| `lightDataAddr` | `LightData` | per frame |

`RenderPerPassData` carries the view, projection, and view-projection matrices, the eye position, and `shadowDataAddr`, the address of this camera's `ShadowRenderViewData` (see [Shadows](#shadows)). Shadow views fill the same struct from their `ShadowView`; their `shadowDataAddr` is left 0.

Every one of these buffers is a persistent-mapped, frame-buffered (`GpuBufferingStrategy::Frame`) BDA buffer, so the CPU writes frame N+1 while the GPU reads frame N. Per-view and per-instance buffers grow on demand.

### Instance data

`RenderInstanceData { gpuIndex, materialSlot }` maps an instance to its world matrix and its material property slot. All views share one array, laid out as each view's opaque entities followed by its transparent entities. A view's `opaqueInstanceDataAddr` and `transparentInstanceDataAddr` point into it. Shaders index it with `SV_VulkanInstanceID`, which includes the draw's `firstInstance`.

---

## Render passes

### What a pass does

Every pass derives from `RenderPass` and implements `render(RenderPassContext)`. The base class provides the shared steps:

1. **`barrierAndBeginRender`** — resolve the view's color and depth attachments, emit the layout barriers, and begin dynamic rendering.
2. **`compileRenderList`** — walk the view's sorted entities and merge consecutive entries with the same mesh, material, and index range into one `RenderPassItem` with an instance count. Because opaque entities are sorted by material then mesh, this turns most of a scene into instanced draws.
3. **`drawRenderList`** — bind each item's pipeline and mesh only when they change, re-push the push constants only when the material properties address changes, and issue one `cmdDrawIndexed` per item with `firstInstance = instanceOffset`.

Two hooks specialize a pass:

- **`resolveGraphicsPipelineHandle`** picks which pipeline variant of the material to use (see [Material variants](#material-variants)).
- **`isValidRenderPassMaterial`** filters materials out of a pass entirely (the shadow pass uses this to skip non-casters).

Push constants survive pipeline binds because every pipeline layout shares the same fixed push-constant range. If that ever changes, `drawRenderList` must re-push on every pipeline change.

### The passes

| Pass | Views | Material variant | Attachments | Load / store | Notes |
|------|-------|------------------|-------------|--------------|-------|
| `ShadowPass` | `Shadow` | `Shadow` | depth: one pool layer | clear / store | Skips materials with `castsShadows == false`. Sets depth bias per view. Afterwards transitions each written layer to shader-read. |
| `DepthOnlyPass` | `DepthOnly` | `DepthOnly` | depth: camera RT | clear / store | Opaque entities only. |
| `ForwardOpaquePass` | `Camera` | `Color` | color + depth: camera RT | clear / color store, depth don't-care | Opaque and masked entities. Lit by `lit.slang`. |
| `ForwardTransparentPass` | `Camera` | `Color` | color + depth: camera RT | load / color store, depth don't-care | Transparent entities, sorted back to front. |
| `BlitPass` | every view with a color or depth source | its own pipeline | swapchain color | clear / store | Exposure, tone mapping, and sRGB encode for color; a linearized-depth visualization for depth-only views. Uses each camera's viewport. |

When there are no views at all, `RenderManager` skips the passes and just clears the swapchain.

### Material variants

A `Material` owns one pipeline per `MaterialVariantType`, created up front:

- **`Color`** — the material's full shader set, scene attachment formats (`RGBA16_SFloat` color + swapchain depth format), and the material's blend state.
- **`DepthOnly`** — vertex stage only, no color attachments, `D32_SFloat` depth.
- **`Shadow`** — same as `DepthOnly`, plus `depthClampEnabled`, `depthBiasEnabled`, and depth bias as dynamic state. Only created when the material casts shadows.

`MaterialDescriptor::castsShadows` defaults to true and is forced to false for transparent materials.

### Scene formats and HDR

Camera render textures use the *scene* formats, `RenderManager::getSceneRenderFormats()`: `RGBA16_SFloat` color and the swapchain's depth format. Lighting is computed and stored in linear HDR. Nothing is clamped until `BlitPass`:

```
hdr color ─► × exposure ─► tone map ─► (sRGB encode, if the swapchain is not an _SRGB format) ─► swapchain
```

The tone mapper and exposure come from `EngineConfiguration::tonemapping` and can be changed at runtime with `RenderManager::setToneMap` / `setToneMapExposure`.

| `ToneMapper` | Behavior | Preset in `tonemapping.hpp` |
|--------------|----------|-----------------------------|
| `None` | `saturate` | `DefaultToneMapNone` (exposure 1.0) |
| `ACES` | ACES fitted (RRT + ODT approximation) | `DefaultToneMapACES` (exposure 2.5) |
| `KronosNeutralPBR` | Khronos PBR Neutral | `DefaultToneMapKhronos` (exposure 2.0) |

`BlitPass` decides at setup whether the swapchain format is sRGB. If it is, the hardware encodes on write and the shader outputs linear; otherwise the shader encodes with `linearToSRGB`.

---

## Lighting

### Directional lights

A directional light is an entity with a `DirectionalLight` component and a `Transform`:

```cpp
struct DirectionalLight
{
    color color{ colors::White };
    float intensity{ 1.0f };
    ShadowBiasSettings shadowBias{};    // used only when overridesDepthBias is true
    bool castsShadows{ false };
    bool overridesDepthBias{ false };
};
```

The light shines along the entity's world-space forward vector. `DirectionalLightSystem` (`LateUpdate`) calls `LightManager::markActive` for every such entity with `toLight = -worldMatrix.forward()`. `markActive` only queues a command into a per-job vector, so the system runs in parallel safely.

Lights are **rebuilt every frame** from these commands. There is no registration step: a light that stops being updated (its entity is destroyed, or loses the component) simply isn't in next frame's list.

### Light data on the GPU

`LightManager::onPreRender` drains the queued commands and uploads two buffers:

```cpp
struct LightData                       // what lightDataAddr points to
{
    color ambientColor;
    uint64_t directionalLightsAddr;
    uint32_t directionalLightsCount;
};

struct DirectionalLightData            // one per active light
{
    vec4 colorIntensity;               // rgb + intensity
    vec3 toLight;
    uint32_t shadowIndex;              // currently always the null index; see ShadowRenderViewData::lightIndex
};
```

The directional array is rewritten every frame. `LightData` is only rewritten when it changes (new address, new count, or `setAmbientColor`), and then for `framesInFlight` frames so every frame-buffered copy catches up.

### Shading

`lit.slang` builds a `SurfaceData` (normal, view vector, base color, roughness, metallic, occlusion) and calls `shadeLights`. The BRDF lives in `brdf.slang`:

- **`prepareBRDF`**, once per fragment: `diffuseColor = baseColor × (1 − metallic)`, `f0 = lerp(0.04, baseColor, metallic)`, `α = max(roughness², 0.002025)`, `NoV = max(|N·V|, 1e-4)`.
- **`evaluateBRDF`**, once per light: GGX/Trowbridge-Reitz D, height-correlated Smith-GGX visibility V (Heitz 2014; a cheaper Hammon 2017 approximation is also provided), Schlick F on `V·H`, and Lambert diffuse weighted by `(1 − F)`, matching the glTF mix. Returns `(diffuse + specular) × radiance × N·L`.

For each directional light, the result is multiplied by that light's shadow factor (see [Sampling](#sampling)). Shadows affect direct light only.

Ambient is a flat `ambientColor × baseColor × (1 − metallic)`. There is no image-based lighting yet, and `SurfaceData::occlusion` is read but not applied.

---

## Shadows

### Scope

- **Directional lights only.** Spot and point shadows are planned. The pool and allocator already have slots for them (`ShadowCasterType::Spot`, `Point`) with capacity 0.
- **One shadowed directional light.** The first active light with `castsShadows` set is the one that gets shadows. Others still light the scene, unshadowed.
- **Per-camera opt-in.** A camera gets shadows when it is active, is a `CameraType::Standard` camera, and has `CameraDescriptor::shadows.enabled` set (`ShadowManager::cameraValidForShadows`). `shadows.cascadeCount` (1 to `MaxShadowCascades`, 4) sets its cascade count. Each camera gets its own cascades, fitted to its own frustum.

### The shadow pool

All directional shadow maps live in **one** texture: a `D32_SFloat` 2D array of `resolution × resolution × directionalLayers`. It is created lazily on the first frame, transitioned straight to shader-read, and registered in the bindless texture table's `Texture2DArray` sub-table (`forcedArray` keeps it an array even with one layer, so shader sampling is always the same).

Each cascade is one layer. A camera with four cascades owns four consecutive layers.

Memory is `resolution² × 4 bytes × layers`: 16 MiB per layer at 2048², so the default of 8 layers is 128 MiB. Size the layer count for the cameras and cascades you actually use, not for the constants' worst case.

### The allocator

`ShadowAllocator` hands out contiguous layer ranges from a `LayerRangeAllocator` per caster type, keyed by `ShadowKey { Entity light; CameraHandle camera; }`. Each frame:

```
beginFrame(frame)                     free ranges that were retired ≥ framesInFlight frames ago
acquire(key, type, count, frame)      for every (light, camera) that wants shadows this frame
releaseUnused(frame)                  retire every key that wasn't acquired this frame
```

- **Stable.** A key that is acquired again with the same type and layer count gets the same range back, so a camera's cascades don't move around the pool.
- **Shape change.** If the layer count changes (e.g. the camera's cascade count), the old range is retired and a new one is allocated.
- **Deferred reuse.** Retired ranges are not reused until `framesInFlight` frames later, so in-flight GPU work never sees a layer handed to a new owner.
- **Exhaustion.** If no range fits, `acquire` fails and that camera renders unshadowed this frame. There is no fallback to fewer cascades.

Keys use `CameraHandle`, not the camera's slot, because the slot changes whenever the camera list is re-sorted.

### Building the cascades

For each shadowed camera, `ShadowManager::onPreRender`:

1. **Splits.** `computeCascadeSplits` divides `[camNear, min(directionalShadowDistance, camFar)]` with the practical split scheme: `lerp(uniform, logarithmic, cascadeSplitLambda)`. `cascadeSplits[i]` is the far view distance of cascade `i`.
2. **Bounding sphere per cascade.** Perspective cameras use `sliceSpherePerspective`, which computes the slice's bounding sphere analytically from fov, aspect, and the slice depths. Orthographic cameras build the slice's corners (`sliceCornersOrthographic`) and bound those (`Sphere::fromCorners`). The radius is quantized, so it doesn't change as the camera rotates. Consecutive slices **overlap** by the cascade blend band (`CascadeBlendFraction`, 10% of the earlier cascade's depth range), so the blend region is inside both cascades.
3. **Light-space fit** (`fitDirectionalLight`):
   - eye = `center + toLight × r`, looking at the center. The up vector falls back to world forward when the light is within ~8° of vertical.
   - render projection: orthographic `[-r, r]²`, depth `[0, 2r]`.
   - **texel snapping:** the projection is offset so the world origin lands on a texel corner. The texel grid is then fixed in world space, so shadow edges don't shimmer as the camera moves.
   - cull projection: the same box with its near plane pulled back toward the light by `directionalCasterPullback`, so casters outside the box (tall geometry between the light and the receivers) are still drawn.
4. **Output.** One `ShadowView` per cascade (cull frustum, matrices, layer, bias), and one `ShadowRenderViewData` for the camera.

Casters in front of the render projection's near plane aren't clipped: the shadow pipeline's depth clamp flattens them onto the near plane, where they still occlude.

### What the shaders get

```cpp
struct ShadowRenderViewData            // one per camera slot; shadowDataAddr points at it
{
    mat4 cascadeViewProj[MaxShadowCascades];
    vec4 cascadeSplits;                // far view distance of each cascade
    vec4 cascadeNormalOffset;          // world-space normal offset per cascade (texel size × normalOffsetTexels)
    float resolution;
    float resolutionInverse;
    uint32_t shadowArrayIndex;         // texture-table index of the pool; null index = unshadowed
    uint32_t firstLayer;
    uint32_t cascadeCount;
    uint32_t lightIndex;               // which directional light this shadows
    uint32_t padding[2];
};
```

The manager uploads one array of `MaxSceneCameras` of these every frame. Cameras without shadows keep the default block, whose null `shadowArrayIndex` tells the shader to skip shadowing. That way every camera view has a valid `shadowDataAddr`.

### Culling and rendering

Each `ShadowView` becomes a `CullView` of type `Shadow` that never wants transparent entities, then a `RenderView` of type `Shadow`. `ShadowPass` renders every shadow view:

- clears and renders the view's layer through the material's `Shadow` variant, with `cmdSetDepthBias` from the view's `ShadowBiasSettings`.
- after all shadow views are drawn, transitions each written layer from depth-attachment to shader-read, before any camera pass samples it.

### Sampling

`calcDirectionalLightShadow` in `lighting.slang`, per light, per fragment:

1. **Gate.** Return 1 (lit) unless the data pointer is valid, `shadowArrayIndex` isn't null, `lightIndex` matches this light, and `cascadeCount > 0`.
2. **Cascade selection.** The first cascade whose split is beyond the fragment's view depth. Past the last split, return 1.
3. **Normal offset.** Move the lookup position along the normal by `cascadeNormalOffset[c] × (1 − N·L)`. This removes most acne without a large depth bias.
4. **Project.** `uv = clip.xy × 0.5 + 0.5` with no divide (orthographic) and no v flip (the Vulkan Y flip is already in the projection). The reference depth is `saturate(clip.z)`.
5. **Filter.** `sampleGlobalShadowTextureArray5x5`: a 5×5 tent PCF (Castaño's optimized form) built from nine bilinear `SampleCmpLevelZero` taps.
6. **Blend.** In the last `CASCADE_BLEND_FRACTION` of each cascade, also sample the next cascade and lerp. In the last cascade the blend fades toward lit, which gives a soft edge at the shadow distance.

The comparison uses `SamplerPredefines::ShadowCompare` (`DefaultSamplerIndex_ShadowCompare`): linear filtering, clamp-to-border with an opaque-black border, compare `GreaterOrEqual`. With reverse-Z, a stored 0 means "nothing in front", so anything outside the map compares as lit.

### Bias

`ShadowBiasSettings`:

| Field | Default | Meaning |
|-------|---------|---------|
| `depthBiasConstantFactor` | −1.0 | raster constant bias; negative because of reverse-Z |
| `depthBiasSlopeFactor` | −1.5 | raster slope-scaled bias; negative because of reverse-Z |
| `depthBiasClamp` | 0.0 | 0 = no clamp |
| `normalOffsetTexels` | 1.0 | receiver normal offset, in shadow-map texels |

`ShadowConfiguration::defaultDirectionalBias` applies to every directional light. A light is meant to override it with `DirectionalLight::shadowBias` + `overridesDepthBias`; see [What's not yet here](#whats-not-yet-here).

### Configuration

`EngineConfiguration::shadow` (`ShadowConfiguration`, validated by `sanitize()`):

| Field | Default | Range | Notes |
|-------|---------|-------|-------|
| `resolution` | 2048 | 32 – 4096 | per layer, shared by every map |
| `directionalLayers` | 8 | 1 – 1024 | pool size; 2 cameras × 4 cascades by default |
| `directionalShadowDistance` | 100 | 1 – 131072 | how far from the camera directional shadows reach |
| `directionalCasterPullback` | 100 | 0 – 131072 | how far toward the light casters are still culled in |
| `cascadeSplitLambda` | 0.75 | 0 – 1 | 0 = uniform splits, 1 = logarithmic |
| `defaultDirectionalBias` | see above | | `defaultSpotBias` / `defaultPointBias` are reserved |

The configuration is read once at setup. `resolution` and `directionalLayers` size the pool texture on the first frame and can't change afterwards.

---

## Conventions and invariants

### Reverse-Z everywhere

Depth is reversed (near = 1, far = 0) in every view, shadow maps included. That means:

- depth attachments clear to **0**, and the depth test is `Greater`.
- the shadow comparison is `GreaterOrEqual`, and the "lit" border value is **0** (opaque black).
- raster depth bias factors are **negative**.
- `Frustum::fromViewProjection` is called with `reverseZ = true`.

### View space is left-handed

View-space +Z points forward, so a fragment's view depth is `mul(view, positionWS).z` and is positive in front of the camera. Cascade splits are distances along that axis.

### C++ ↔ shader structs move together

These are declared on both sides and must match exactly. Each C++ struct is padded to 16 bytes and has a `static_assert` on its size:

| C++ | Slang |
|-----|-------|
| `RenderPerFrameData`, `RenderPerPassData`, `RenderInstanceData`, `RenderPushConstants` (`renderStructs.hpp`) | `input.slang` |
| `LightData`, `DirectionalLightData` | `lighting.slang` |
| `ShadowRenderViewData` | `lighting.slang` |
| `BlitPushConstants` (`blitPass.cpp`) | `blit.slang` |
| `ToneMapper` values | `tonemapping.slang` |
| `constants::MaxShadowCascades` | `MAX_SHADOW_CASCADES` |
| `CascadeBlendFraction` (`shadowManager.cpp`) | `CASCADE_BLEND_FRACTION` |

`MaxShadowCascades` is also capped at 4 because cascade splits and normal offsets travel as `vec4`s.

### Per-frame data is frame-buffered

Anything the CPU writes every frame lives in a `GpuBufferingStrategy::Frame` buffer and is `swapBuffers`'d to the current frame-in-flight index before it is written. Shadow layers follow the same rule through the allocator's deferred reuse.

### Decide before Render

Views, shadow allocations, cascade fits, and light lists are all fixed before the `Render` callback. Passes only read them. Adding a new kind of view means adding it in `PreRender` (for culling) and teaching `writeViewData` how to fill its `RenderPerPassData` and `ResolvedView`.

---

## What's not yet here

- **Spot and point light shadows.** The caster types, pool slots, and `MaxCullViews` budget exist; the pools have zero capacity and nothing allocates from them.
- **More than one shadowed directional light.** `ShadowRenderViewData` is one block per camera. Supporting more would make it one block per (camera, light).
- **Alpha-masked shadows.** The `Shadow` and `DepthOnly` variants are vertex-only, so masked materials cast (and depth-test) as if fully opaque.
- **Per-light bias override.** `DirectionalLight::shadowBias` / `overridesDepthBias` aren't carried into the light list yet, so every light uses `defaultDirectionalBias`.
- **Image-based lighting and ambient occlusion.** Ambient is a flat color, and the occlusion value is unused.
- **A post-processing stack.** Exposure and tone mapping are a fixed step in `BlitPass`; a configurable stack is planned for v0.5, along with deferred rendering.
- **Shadow debug views.** Cascade visualization is a commented-out block in `lit.slang`, not a runtime toggle.
- **Light direction lags by a frame.** `DirectionalLightSystem` runs in `LateUpdate` and reads world matrices that are only recomputed in `PreRender`, so the light uses last frame's transform. Invisible for a static sun, but visible on a light rotated every frame.
- **`DirectionalLightData::shadowIndex`** is reserved and always null; the camera → light link is `ShadowRenderViewData::lightIndex`.

---

## Useful files to read

| File | What lives here |
|------|-----------------|
| `litl/engine/src/engineCallbacks.cpp` | The `PreRender` pre-group ordering (lights → scene → shadows → materials) |
| `litl/engine/src/render/renderManager.cpp` | `onRender`, view gathering, per-view data upload, pass order |
| `litl/engine/include/litl-engine/render/renderView.hpp` | `RenderView`, `ResolvedView`, `RenderViewType` |
| `litl/engine/include/litl-engine/render/renderStructs.hpp` | Push constants and per-frame / per-pass / per-instance structs |
| `litl/engine/src/render/renderPass.cpp` | Render target resolution, render list batching, the draw loop |
| `litl/engine/src/render/passes/*.cpp` | The five passes |
| `litl/engine/src/objects/material/material.cpp` | `Color` / `DepthOnly` / `Shadow` pipeline variants |
| `litl/engine/src/ecs/systems/cullingSystem.cpp` | Camera and shadow `CullView`s, per-view visibility |
| `litl/engine/src/lights/lightManager.cpp` | Light commands, light buffers |
| `litl/engine/src/lights/directional/directionalLightSystem.cpp` | Where a light's direction comes from |
| `litl/engine/src/lights/shadows/shadowManager.cpp` | Pool creation, cascade splits, fitting, texel snapping, upload |
| `litl/engine/src/lights/shadows/shadowAllocator.cpp` | Layer-range allocation, retirement, deferred reuse |
| `litl/engine/include/litl-engine/lights/shadows/shadowConfiguration.hpp` | Shadow settings and their ranges |
| `litl/core/src/litl-core/math/shadows.cpp` | Cascade splits and slice bounds (covered by `tests/src/litl-core/math/shadowsMath_tests.cpp`) |
| `assets/litl/shaders/lit.slang` | The lit material: surface setup, shadow inputs |
| `assets/litl/shaders/litl/lighting.slang` | Light structs, shadow gating, cascade selection and blending |
| `assets/litl/shaders/litl/brdf.slang` | The BRDF |
| `assets/litl/shaders/litl/textures.slang` | Texture tables, the shadow-compare alias, 5×5 PCF |
| `assets/litl/shaders/blit.slang`, `litl/tonemapping.slang` | Exposure, tone mapping, sRGB encode, depth visualization |