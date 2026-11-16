#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/lights/shadows/shadowManager.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/scene/sceneView.hpp"

namespace litl
{
    std::array<CullView, constants::MaxCullViews> CullingSystem::s_views{};
    constinit uint32_t CullingSystem::s_viewCount{ 0u };

    std::array<CullingBucket, constants::max_thread_count> CullingSystem::s_cullingBuckets{};
    CullingBucket CullingSystem::s_combinedBucket{};
    constinit bool CullingSystem::s_combined{ false };

    bool isCameraCullable(Camera const& camera) noexcept
    {
        return camera.getIsActive() && camera.getRenderTextureHandle().isValid();
    }

    // -------------------------------------------------------------------------------------
    // Setup / Prepare
    // -------------------------------------------------------------------------------------

    void CullingSystem::setup(ServiceProvider& services)
    {
        m_pObjectPool = services.get<ObjectPool>();
        m_pSceneView = services.get<SceneView>();
        m_pShadowManager = services.get<ShadowManager>();

        LITL_FATAL_ASSERT_MSG((m_pObjectPool != nullptr), "Failed to inject ObjectPool into CullingSystem");
        LITL_FATAL_ASSERT_MSG((m_pSceneView != nullptr), "Failed to inject SceneView into CullingSystem");
        LITL_FATAL_ASSERT_MSG((m_pShadowManager != nullptr), "Failed to inject ShadowManager into CullingSystem");
    }

    void CullingSystem::prepare()
    {
        reset();

        // 1. Gather this frame's views.

        auto cameras = m_pSceneView->getCameras();

        for (uint32_t slot = 0u; slot < cameras.size(); ++slot)
        {
            Camera* camera = cameras[slot];

            if ((camera == nullptr) || !isCameraCullable(*camera))
            {
                continue;
            }

            addView(CullView{
                .frustum = camera->getFrustum(),
                .camera = camera,
                .sourceIndex = slot,
                .type = CullViewType::Camera,
                .wantsTransparent = (camera->getCameraType() != CameraType::DepthOnly)
            });
        }

        if (m_pShadowManager != nullptr)
        {
            const auto shadowViews = m_pShadowManager->getShadowViews();

            for (uint32_t i = 0u; i < static_cast<uint32_t>(shadowViews.size()); ++i)
            {
                addView(CullView{
                    .frustum = shadowViews[i].cullFrustum,
                    .camera = nullptr,
                    .sourceIndex = i,
                    .type = CullViewType::Shadow,
                    .wantsTransparent = false
                });
            }
        }

        // 2. Query each view and fold the results into the per-entity visibility masks.
        //    The frustum query returns every entity with a transform, so renderables are a subset of this.

        for (uint32_t viewIndex = 0u; viewIndex < s_viewCount; ++viewIndex)
        {
            m_queryScratch.clear();
            m_pSceneView->query(s_views[viewIndex].frustum, m_queryScratch, false);
            markVisible(viewIndex, m_queryScratch);
        }
    }

    bool CullingSystem::addView(CullView const& view) noexcept
    {
        LITL_ASSERT_MSG((s_viewCount < constants::MaxCullViews), "CullingSystem exceeded MaxCullViews. View dropped.", false);

        s_views[s_viewCount] = view;
        s_viewCount += 1u;

        return true;
    }

    void CullingSystem::markVisible(uint32_t viewIndex, std::span<PartitionQueryResult const> results) noexcept
    {
        for (const auto& result : results)
        {
            if (result.entity.isNull())
            {
                continue;
            }

            const uint32_t index = result.entity.index;

            if (index >= m_visibility.size())
            {
                m_visibility.resize(static_cast<size_t>(index) + 1u);
            }

            auto& slot = m_visibility[index];

            if (slot.entity != result.entity)
            {
                // First view to see this entity this frame. reset() nulls touched slots, so a match
                // here always means "already touched this frame", never a stale previous frame.
                slot.entity = result.entity;
                slot.worldPosition = result.worldPosition;
                slot.views.reset();

                m_touchedSlots.push_back(index);
            }

            slot.views[viewIndex] = true;
        }
    }

    // -------------------------------------------------------------------------------------
    // Reset
    // -------------------------------------------------------------------------------------

    void CullingSystem::reset() noexcept
    {
        // Clear, but don't destroy, the containers. They reach a high-water mark and reallocations stop.

        for (const uint32_t index : m_touchedSlots)
        {
            m_visibility[index].entity = Entity::null();
            m_visibility[index].views.reset();
        }

        m_touchedSlots.clear();

        // Clear using last frame's view count before it is reset.
        s_combinedBucket.reset(s_viewCount);

        for (auto& cullingBucket : s_cullingBuckets)
        {
            cullingBucket.reset(s_viewCount);
        }

        s_viewCount = 0u;
        s_combined = false;
    }

    void CullingBucket::reset(uint32_t viewCount) noexcept
    {
        for (uint32_t i = 0u; i < viewCount; ++i)
        {
            views[i].opaque.clear();
            views[i].transparent.clear();
        }
    }

    // -------------------------------------------------------------------------------------
    // Update
    // -------------------------------------------------------------------------------------

    void CullingSystem::update(SystemData const& data, Entity entity, Transform const& /*transform*/, MeshRef const& meshRef, MaterialRef const* materialRef, VariableMaterialsRef const* variableMaterialsRef)
    {
        if (!meshRef.handle.isValid())
        {
            return;
        }

        if (((materialRef == nullptr) || !materialRef->handle.isValid()) && ((variableMaterialsRef == nullptr) || !variableMaterialsRef->handle.isValid()))
        {
            return;
        }

        if (entity.index >= m_visibility.size())
        {
            return;     // Not returned by any view's query this frame.
        }

        const EntityVisibility& visibility = m_visibility[entity.index];

        if ((visibility.entity != entity) || visibility.views.none())
        {
            return;     // Not visible to any view (or the slot belongs to a different version of this index).
        }

        // Material lookups happen once per entity/submesh, then the result is fanned out to each visible view.

        // Use of a MaterialRef indicates that the entire mesh is drawn with the same material.
        if (materialRef != nullptr)
        {
            const Material* material = m_pObjectPool->getMaterial(materialRef->handle);

            if (material != nullptr)
            {
                pushRenderable(data.threadIndex, visibility, RenderableEntity{
                    .entity = entity,
                    .meshRef = meshRef,
                    .materialRef = *materialRef,
                    .materialBlendMode = material->getBlendMode(),
                    .position = visibility.worldPosition,
                    .distanceSquared = 0.0f,                            // Per-view, set in pushRenderable.
                    .firstIndex = 0u,
                    .indexCount = constants::uint32_null_index
                    });
            }
        }
        // Use of a VariableMaterialsRef indicates that only the submeshes with a matching material index are rendered.
        else if (variableMaterialsRef != nullptr)
        {
            auto* mesh = m_pObjectPool->getMesh(meshRef.handle);
            auto* materialBindings = m_pObjectPool->getMaterialBindings(variableMaterialsRef->handle);

            if ((mesh == nullptr) || (materialBindings == nullptr))
            {
                return;
            }

            const auto& submeshes = mesh->getGeoMesh().getSubmeshes();
            const auto& bindings = materialBindings->getBindings();

            for (size_t i = 0u; i < submeshes.size(); ++i)
            {
                if (submeshes[i].materialSlot >= bindings.size())
                {
                    continue;
                }

                const auto& binding = bindings[submeshes[i].materialSlot];
                const Material* material = m_pObjectPool->getMaterial(binding.handle);

                if ((material == nullptr) || !binding.slot.isValid())
                {
                    continue;
                }

                pushRenderable(data.threadIndex, visibility, RenderableEntity{
                    .entity = entity,
                    .meshRef = meshRef,
                    .materialRef = MaterialRef{.handle = binding.handle, .slot = binding.slot },
                    .materialBlendMode = material->getBlendMode(),
                    .position = visibility.worldPosition,
                    .distanceSquared = 0.0f,
                    .firstIndex = submeshes[i].firstIndex,
                    .indexCount = submeshes[i].indexCount
                    });
            }
        }
    }

    void CullingSystem::pushRenderable(uint32_t threadIndex, EntityVisibility const& visibility, RenderableEntity renderable) noexcept
    {
        auto& bucket = s_cullingBuckets[threadIndex];
        const bool isTransparent = (renderable.materialBlendMode == MaterialBlendMode::Transparent);

        for (uint32_t viewIndex = 0u; viewIndex < s_viewCount; ++viewIndex)
        {
            if (!visibility.views[viewIndex])
            {
                continue;
            }

            const CullView& view = s_views[viewIndex];

            if (isTransparent && !view.wantsTransparent)
            {
                continue;
            }

            renderable.distanceSquared = view.frustum.getOrigin().distanceSqTo(visibility.worldPosition);

            auto& viewEntities = bucket.views[viewIndex];
            (isTransparent ? viewEntities.transparent : viewEntities.opaque).push_back(renderable);
        }
    }

    // -------------------------------------------------------------------------------------
    // Results
    // -------------------------------------------------------------------------------------

    CullingBucket& CullingSystem::getCombinedCullingBucket() noexcept
    {
        if (!s_combined)
        {
            for (uint32_t viewIndex = 0u; viewIndex < s_viewCount; ++viewIndex)
            {
                auto& combined = s_combinedBucket.views[viewIndex];

                size_t totalOpaque = 0u;
                size_t totalTransparent = 0u;

                for (const auto& threadBucket : s_cullingBuckets)
                {
                    totalOpaque += threadBucket.views[viewIndex].opaque.size();
                    totalTransparent += threadBucket.views[viewIndex].transparent.size();
                }

                combined.opaque.reserve(totalOpaque);
                combined.transparent.reserve(totalTransparent);

                for (const auto& threadBucket : s_cullingBuckets)
                {
                    const auto& opaque = threadBucket.views[viewIndex].opaque;
                    const auto& transparent = threadBucket.views[viewIndex].transparent;

                    combined.opaque.insert(combined.opaque.end(), opaque.begin(), opaque.end());
                    combined.transparent.insert(combined.transparent.end(), transparent.begin(), transparent.end());
                }
            }

            s_combined = true;
        }

        return s_combinedBucket;
    }

    std::span<CullView const> CullingSystem::getCullViews() noexcept
    {
        return std::span<CullView const>(s_views.data(), s_viewCount);
    }
}