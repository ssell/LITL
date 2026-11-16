#include <array>
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
#include "litl-engine/lights/shadows/shadowRenderViewData.hpp"
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
            auto config = services.get<Configuration>();

            LITL_FATAL_ASSERT_MSG((objectPool != nullptr), "Failed to inject ObjectPool into ShadowManager");
            LITL_FATAL_ASSERT_MSG((lightManager != nullptr), "Failed to inject LightManager into ShadowManager");
            LITL_FATAL_ASSERT_MSG((sceneView != nullptr), "Failed to inject SceneView into ShadowManager");
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

            const TextureDescriptor directionalShadowTextureDesc{
                .objectInfo = ObjectDescriptor {.name = "LITL_INTERNAL_TEXTURE_DirectionalShadowArray" },
                .textureInfo = TextureResourceDescriptor {
                    .width = shadowConfig.resolution,
                    .height = shadowConfig.resolution,
                    .format = DataFormat::D32_SFloat,
                    .usage = TextureUsageFlagBits::DepthStencilAttachment | TextureUsageFlagBits::Sampled,      // Render depth to it, and sample from a shader.
                    .arrayLayers = shadowConfig.directionalLayers,
                    .residesInTextureTable = true
                }
            };

            directionalShadowTextureHandle = objectPool->createTexture(directionalShadowTextureDesc);

            LITL_FATAL_ASSERT_MSG(directionalShadowTextureHandle.isValid(), "Failed to create directional shadow array texture.");

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

    std::span<ShadowView const> ShadowManager::getShadowViews() const noexcept
    {
        return m_pImpl->shadowViews;
    }

    bool ShadowManager::cameraValidForShadows(Camera const* camera) noexcept
    {
        return (camera != nullptr) &&
            camera->getIsActive() &&
            (camera->getCameraType() == CameraType::Standard) &&
            camera->getShadowsEnabled();
    }
}