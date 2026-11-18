#include <array>
#include <optional>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/math/types.hpp"
#include "litl-core/math/bounds.hpp"
#include "litl-core/math/shadows.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/config.hpp"
#include "litl-engine/constants.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-engine/lights/lightManager.hpp"
#include "litl-engine/lights/shadows/shadowManager.hpp"
#include "litl-engine/lights/shadows/shadowAllocator.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    namespace
    {
        struct ShadowedLight
        {
            uint32_t index{ constants::uint32_null_index };
            Entity entity{};
            vec3 toLight{};
            ShadowBiasSettings bias{};
            bool biasOverride{ false };
        };

        struct DirectionalLightFit
        {
            mat4 view{};
            mat4 proj{};
            mat4 viewProj{};
            mat4 cullViewProj{};
            vec3 eye{};
        };

        [[nodiscard]] DirectionalLightFit fitDirectionalLight(vec3 toLight, bounds::Sphere const& sphere, uint32_t resolution, float casterPullback) noexcept
        {
            DirectionalLightFit fit{};
            
            const float r = sphere.radius;
            const vec3 forward = normalize(toLight * -1.0f);
            const vec3 up = (abs(dot(forward, vec3::up())) > 0.99f) ? vec3::forward() : vec3::up();     // Degenerate when the light points (anti)parallel to +Y.

            fit.eye = sphere.center + (toLight * r);
            fit.view = mat4::lookAt(fit.eye, sphere.center, up);
            fit.proj = mat4::orthographic(-r, r, -r, r, 0.0f, 2.0f * r);                                // Tight depth range for precision. Casters in front of the eye are pancaked onto the near plane by depthClampEnable in the shadow pipeline.
            
            // Texel snapping: shift the projection so the world origin lands on a texel center. Ortho => w == 1, no divide needed.
            const vec4 origin = (fit.proj * fit.view) * vec4(0.0f, 0.0f, 0.0f, 1.0f);
            const float half = static_cast<float>(resolution) * 0.5f;
            const float sx = origin.x() * half;
            const float sy = origin.y() * half;
            const float dx = (std::round(sx) - sx) / half;
            const float dy = (std::round(sy) - sy) / half;

            fit.proj[3][0] += dx;
            fit.proj[3][1] += dy;
            fit.viewProj = fit.proj * fit.view;

            // Culling box: same view, near plane pushed back toward the light so casters
            // outside the receiver box (tall pillars, the sky-side of the roof) are not culled.
            const mat4 cullProj = mat4::orthographic(-r, r, -r, r, -casterPullback, 2.0f * r);
            fit.cullViewProj = cullProj * fit.view;

            return fit;
        }
    }

    struct ShadowManager::Impl
    {
        std::shared_ptr<ObjectPool> objectPool;
        std::shared_ptr<LightManager> lightManager;
        std::shared_ptr<SceneView> sceneView;
        std::shared_ptr<RenderManager> renderManager;

        ShadowConfiguration shadowConfig{};
        ShadowAllocator allocator{};

        TextureHandle directionalShadowTextureHandle{};
        GpuBufferHandle shadowDataBufferHandle{};
        uint64_t shadowDataAddress{ 0ull };

        std::array<ShadowRenderViewData, constants::MaxSceneCameras> cameraShadowData{};
        std::vector<ShadowView> shadowViews;

        bool initialized{ false };

        // ---------------------------------------------------------------------------------
        // Lifecycle
        // ---------------------------------------------------------------------------------

        void setup(ServiceProvider& services) noexcept
        {
            objectPool = services.get<ObjectPool>();
            lightManager = services.get<LightManager>();
            sceneView = services.get<SceneView>();
            renderManager = services.get<RenderManager>();
            auto config = services.get<Configuration>();

            LITL_FATAL_ASSERT_MSG((objectPool != nullptr), "Failed to inject ObjectPool into ShadowManager");
            LITL_FATAL_ASSERT_MSG((lightManager != nullptr), "Failed to inject LightManager into ShadowManager");
            LITL_FATAL_ASSERT_MSG((sceneView != nullptr), "Failed to inject SceneView into ShadowManager");
            LITL_FATAL_ASSERT_MSG((renderManager != nullptr), "Failed to inject RenderManager into ShadowManager");
            LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into ShadowManager");
            
            shadowConfig = config->shadow;
            shadowViews.reserve(constants::MaxShadowCameras * constants::MaxShadowCascades);
        }

        void destroy() noexcept
        {
            if (directionalShadowTextureHandle.isValid())
            {
                objectPool->destroyTexture(directionalShadowTextureHandle);
                directionalShadowTextureHandle = {};
            }

            shadowViews.clear();
            initialized = false;
        }

        void initialize(FrameData const& frame) noexcept
        {
            allocator.setup(ShadowPoolCapacities{
                .directionalLayers = shadowConfig.directionalLayers,
                .spotLayers = 0u,                                           // TODO add support for spot light shadows
                .pointCubes = 0u                                            // TODO add support for point light shadows
            }, frame.framesInFlight);

            // -----------------------------------------------------------------------------
            // Create the Directional Shadow Pool Texture

            const TextureDescriptor directionalShadowTextureDesc{
                .objectInfo = ObjectDescriptor {.name = "LITL_INTERNAL_TEXTURE_DirectionalShadowArray" },
                .textureInfo = TextureResourceDescriptor {
                    .width = shadowConfig.resolution,
                    .height = shadowConfig.resolution,
                    .format = DataFormat::D32_SFloat,
                    .usage = TextureUsageFlagBits::DepthStencilAttachment | TextureUsageFlagBits::Sampled,      // Render depth to it, and sample from a shader.
                    .arrayLayers = shadowConfig.directionalLayers,
                    .forcedArray = true,                                                                        // Place the texture in the array table even if layers/cascades is just 1 to keep the shader sampling consistent.
                    .residesInTextureTable = true
                }
            };

            directionalShadowTextureHandle = objectPool->createTexture(directionalShadowTextureDesc);
            auto* directionalShadowTexture = objectPool->getTexture(directionalShadowTextureHandle);

            LITL_FATAL_ASSERT_MSG((directionalShadowTexture != nullptr), "Failed to create directional shadow array texture.");

            // -----------------------------------------------------------------------------
            // Barrier the Shadow Texture from Undefined to Shader Read

            {
                auto* renderer = renderManager->getRenderer();
                auto scopedCommandBuffer = renderer->createScopedCommandBuffer();
                renderer->cmdPipelineBarrier(scopedCommandBuffer.get(), PipelineBarrierCommand::copyWithTexture(PipelineBarrierUndefinedToShaderRead, directionalShadowTexture->getTextureResourceHandle(), 0u, shadowConfig.directionalLayers));
            }

            // -----------------------------------------------------------------------------
            // Create the Shadow Data GPU Buffer

            const GpuBufferDescriptor shadowDataBufferDesc{
                .objectInfo = ObjectDescriptor {.name = "LITL_INTERNAL_BUFFER_ShadowViewData" },
                .type = BufferTypeFlagBits::BufferDeviceAddress,
                .memoryUsage = BufferMemoryUsage::PersistentMap,
                .bufferStrategy = GpuBufferingStrategy::Frame,
                .bytes = sizeof(ShadowRenderViewData) * cameraShadowData.size(),
                .itemBytes = sizeof(ShadowRenderViewData)
            };

            shadowDataBufferHandle = objectPool->createGpuBuffer(shadowDataBufferDesc);

            LITL_FATAL_ASSERT_MSG(shadowDataBufferHandle.isValid(), "Failed to create shadow data buffer.");

            initialized = true;
        }

        // ---------------------------------------------------------------------------------
        // Per Frame Update
        // ---------------------------------------------------------------------------------

        void onPreRender(FrameData const& frame) noexcept
        {
            if (!initialized)
            {
                initialize(frame);
            }

            shadowViews.clear();
            cameraShadowData.fill(ShadowRenderViewData{});
            allocator.beginFrame(frame.frameCount);

            if (const auto directionalLight = selectShadowedDirectionalLight(); directionalLight.has_value())
            {
                auto cameras = sceneView->getCameras();
                const uint32_t slotCount = min(static_cast<uint32_t>(cameras.size()), constants::MaxSceneCameras);

                for (uint32_t slot = 0u; slot < slotCount; ++slot)
                {
                    const Camera* camera = cameras[slot];

                    if (ShadowManager::cameraValidForShadows(camera))
                    {
                        buildCameraDirectionalShadows(slot, *camera, *directionalLight, frame.frameCount);
                    }
                }
            }
            
            // Anything not acquired this frame(camera disabled shadows, light stopped casting, ...) is retired and becomes reusable framesInFlight frames from now.
            allocator.releaseUnused(frame.frameCount);
            upload(frame.frameInFlightIndex);
        }

        /// <summary>
        /// Returns the first directional light that is marked as casting shadows.
        /// We currently only support a single directional cascaded shadow caster.
        /// </summary>
        [[nodiscard]] std::optional<ShadowedLight> selectShadowedDirectionalLight() const noexcept
        {
            const auto lights = lightManager->getDirectionalLightData();

            for (uint32_t i = 0u; i < static_cast<uint32_t>(lights.size()); ++i)
            {
                if (lights[i].castsShadows)
                {
                    return ShadowedLight{ 
                        .index = i, 
                        .entity = lights[i].entity, 
                        .toLight = lights[i].data.toLight,
                        .bias = lights[i].shadowBias,
                        .biasOverride = lights[i].biasOverride
                    };
                }
            }

            return std::nullopt;
        }

        static_assert(constants::MaxShadowCascades <= 4, "constants::MaxShadowCascades must be less or equal to 4 as cascade biases are passed as float4");

        void buildCameraDirectionalShadows(uint32_t slot, Camera const& camera, ShadowedLight const& light, uint32_t frameCount) noexcept
        {
            const uint32_t cascadeCount = clamp(camera.getDescriptor().shadows.cascadeCount, 1u, constants::MaxShadowCascades);

            const auto acquired = allocator.acquire(
                ShadowKey{ .light = light.entity, .cameraSlot = slot },
                ShadowCasterType::Directional,
                cascadeCount,
                frameCount
            );

            if (!acquired.has_value())
            {
                // Pool exhausted. Camera stays at the sentinel (unshadowed) this frame.
                return;
            }

            const Texture* texture = objectPool->getTexture(directionalShadowTextureHandle);

            if (texture == nullptr)
            {
                return;
            }

            const float camNear = camera.getNear();
            const float camFar = camera.getFar();
            const float shadowFar = min(shadowConfig.directionalShadowDistance, camFar);

            std::array<float, constants::MaxShadowCascades> cascadeSplits{};

            computeCascadeSplits(camNear, shadowFar, cascadeCount, shadowConfig.cascadeSplitLambda, cascadeSplits);

            const auto frustum = camera.getFrustum();
            const auto corners = bounds::Frustum::extractCorners(frustum);
            auto& shadowData = cameraShadowData[slot];

            shadowData.shadowArrayIndex = texture->getTextureTableIndex();
            shadowData.firstLayer = acquired->allocation.range.first;
            shadowData.cascadeCount = cascadeCount;
            shadowData.lightIndex = light.index;

            float sliceNear = camNear;
            const ShadowBiasSettings bias = (light.biasOverride ? light.bias : shadowConfig.defaultDirectionalBias);

            const auto getFrustumSphere = [&camera, &frustum, &corners, &camNear, &sliceNear](float sliceFar) -> bounds::Sphere
                {
                    if (camera.getProjection() == CameraProjection::Perspective)
                    {
                        const auto slice = sliceCornersPerspective(corners, camera.getWorldPosition(), camNear, sliceNear, sliceFar);
                        return bounds::Sphere::fromCorners(slice);
                    }
                    else
                    {
                        const auto slice = sliceCornersOrthographic(frustum, camNear, sliceNear, sliceFar);
                        return bounds::Sphere::fromCorners(slice);
                    }
                };
            
            for (uint32_t cascadeIndex = 0u; cascadeIndex < cascadeCount; ++cascadeIndex)
            {
                const auto sphere = getFrustumSphere(cascadeSplits[cascadeIndex]);
                const auto fit = fitDirectionalLight(light.toLight, sphere, shadowConfig.resolution, shadowConfig.directionalCasterPullback);
                const float texelWorld = (2.0f * sphere.radius) / static_cast<float>(shadowConfig.resolution);

                shadowData.cascadeViewProj[cascadeIndex] = fit.viewProj;
                shadowData.cascadeNormalOffset[cascadeIndex] = texelWorld * bias.normalOffsetTexels;

                shadowViews.push_back(ShadowView{
                    .cullFrustum = bounds::Frustum::fromViewProjection(fit.cullViewProj, bounds::FrustumExtractionOptions{ .reverseZ = true }),
                    .viewMatrix = fit.view,
                    .projMatrix = fit.proj,
                    .viewProjMatrix = fit.viewProj,
                    .eyePosition = fit.eye,
                    .type = ShadowCasterType::Directional,
                    .layer = acquired->allocation.range.first + cascadeIndex,
                    .cameraSlot = slot,
                    .cascadeIndex = cascadeIndex,
                    .resolution = shadowConfig.resolution,
                    .bias = (light.biasOverride ? light.bias : shadowConfig.defaultDirectionalBias)
                });

                sliceNear = cascadeSplits[cascadeIndex];
            }

            shadowData.cascadeSplits = vec4(cascadeSplits[0], cascadeSplits[1], cascadeSplits[2], cascadeSplits[3]);
        }

        void upload(uint32_t frameInFlightIndex) noexcept
        {
            GpuBuffer* buffer = objectPool->getGpuBuffer(shadowDataBufferHandle);

            if (buffer == nullptr)
            {
                shadowDataAddress = 0ull;
                return;
            }

            const GpuBufferChunk bufferWrite{
                .sourcePtr = generic_as_byte_span(cameraShadowData.data(), sizeof(ShadowRenderViewData) * cameraShadowData.size()),
                .offset = 0ull
            };

            buffer->swapBuffers(frameInFlightIndex);
            buffer->recordChunkDataWrite(bufferWrite);
            
            shadowDataAddress = buffer->getBufferDeviceAddress().value();
        }
    };

    ShadowManager::ShadowManager()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    ShadowManager::~ShadowManager()
    {
        // ... required as this is stored in a unique_ptr in the service provider ...
    }

    void ShadowManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_pImpl->setup(services);
    }

    void ShadowManager::destroy(Authority<Engine> auth) noexcept
    {
        m_pImpl->destroy();
    }

    void ShadowManager::onPreRender(Authority<EngineCallbacks> auth, FrameData const& frame) noexcept
    {
        m_pImpl->onPreRender(frame);
    }

    bool ShadowManager::cameraValidForShadows(Camera const* camera) noexcept
    {
        return (camera != nullptr) &&
            camera->getIsActive() &&
            (camera->getCameraType() == CameraType::Standard) &&
            camera->getShadowsEnabled();
    }

    std::span<ShadowView const> ShadowManager::getShadowViews() const noexcept
    {
        return m_pImpl->shadowViews;
    }

    ShadowView const* ShadowManager::getShadowView(uint32_t index) const noexcept
    {
        if (index < m_pImpl->shadowViews.size())
        {
            return &m_pImpl->shadowViews[index];
        }
        else
        {
            return nullptr;
        }
    }

    ShadowRenderViewData const& ShadowManager::getCameraShadowData(uint32_t cameraSlot) const noexcept
    {
        static constexpr ShadowRenderViewData unshadowed{};
        return (cameraSlot < m_pImpl->cameraShadowData.size()) ? m_pImpl->cameraShadowData[cameraSlot] : unshadowed;
    }

    uint64_t ShadowManager::getCameraShadowDataAddress(uint32_t cameraSlot) const noexcept
    {
        if((m_pImpl->shadowDataAddress == 0ull) || (cameraSlot >= m_pImpl->cameraShadowData.size()))
        {
            return 0ull;
        }

        return m_pImpl->shadowDataAddress + (static_cast<uint64_t>(cameraSlot) * sizeof(ShadowRenderViewData));
    }

    TextureHandle ShadowManager::getDirectionalShadowTexture() const noexcept
    {
        return m_pImpl->directionalShadowTextureHandle;
    }
}