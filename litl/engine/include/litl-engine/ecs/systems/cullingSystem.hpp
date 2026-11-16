#ifndef LITL_ENGINE_ECS_CULLING_SYSTEM_H__
#define LITL_ENGINE_ECS_CULLING_SYSTEM_H__

#include <array>
#include <bitset>
#include <memory>
#include <span>
#include <vector>
#include <unordered_set>

#include "litl-core/containers/flatHashSet.hpp"
#include "litl-core/math/bounds/frustum.hpp"
#include "litl-engine/render/viewRenderEntities.hpp"
#include "litl-engine/scene/sceneCameras.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"

namespace litl
{
    class ObjectPool;
    class ServiceProvider;
    class EntityCommands;
    class SceneView;
    class Camera;
    class ShadowManager;

    /// <summary>
    /// The type of view being culled.
    /// </summary>
    enum class CullViewType : uint8_t
    {
        Camera = 0u,
        Shadow = 1u
    };

    struct CullView
    {
        bounds::Frustum frustum{};                                  // Frustum of the view
        Camera* camera{ nullptr };                                  // Non-owning camera pointer, valid for the current frame only.
        uint32_t sourceIndex{ constants::uint32_null_index };       // Camera slot or index into ShadowManager
        CullViewType type{ CullViewType::Camera };                  // The view type
        bool wantsTransparent{ true };                              // False for shadow views and depth-only cameras
    };

    /// <summary>
    /// A bucket of visible and renderable entities for each view.
    /// </summary>
    struct alignas(constants::cache_line_size) CullingBucket
    {
        std::array<ViewRenderEntities, constants::MaxCullViews> viewRenderableEntities;
        void reset(uint32_t viewCount) noexcept;
    };

    /// <summary>
    /// Responsible for compiling a list of all renderable entities visible to each camera.
    /// </summary>
    class CullingSystem
    {
        using ViewMask = std::bitset<constants::MaxCullViews>;

        /// <summary>
        /// Per-entity visibility for the current frame. Indexed by Entity::index.
        /// A slot is valid for an entity only if slot.entity == entity (index and version).
        /// </summary>
        struct EntityVisibility
        {
            Entity entity{};
            vec3 worldPosition{};
            ViewMask views{};
        };

    public:

        /// <summary>
        /// Injects the required services.
        /// </summary>
        void setup(ServiceProvider& services);

        /// <summary>
        /// Resets the culling system state and computes the visible entities for each camera.
        /// The visible entities are later checked against our renderable entities to determine visible renderable entities.
        /// </summary>
        void prepare();

        /// <summary>
        /// Checks each renderable entity against the pre-computed set of visible entities for each camera.
        /// If a renderable entity is visible, it is added to a thread-specific collection that is later combined.
        /// </summary>
        void update(SystemData const& data, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const* materialRef, VariableMaterialsRef const* variableMaterialsRef);

        /// <summary>
        /// Retrieves all visible renderable entities for each camera.
        /// Will combine the per-thread results if they have not already been combined.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] static CullingBucket& getCombinedCullingBucket() noexcept;

        /// <summary>
        /// List of all cull views processed by the cull system.
        /// </summary>
        [[nodiscard]] static std::span<CullView const> getCullViews() noexcept;

    private:

        void reset() noexcept;
        bool addView(CullView const& view) noexcept;
        void markVisible(uint32_t viewIndex, std::span<PartitionQueryResult const> results) noexcept;
        void pushRenderable(uint32_t threadIndex, EntityVisibility const& visibility, RenderableEntity renderable) noexcept;

        std::shared_ptr<ObjectPool> m_pObjectPool;
        std::shared_ptr<SceneView> m_pSceneView;
        std::shared_ptr<ShadowManager> m_pShadowManager;

        std::vector<EntityVisibility> m_visibility;             // Indexed by Entity::index. Grows to the highest visible index and stays there.
        std::vector<uint32_t> m_touchedSlots;                   // m_visibility indices written this frame, so reset() is O(visible) not O(capacity)
        std::vector<PartitionQueryResult> m_queryScratch;       // Reused for each views partition query.

        static std::array<CullView, constants::MaxCullViews> s_views;
        static uint32_t s_viewCount;

        static std::array<CullingBucket, constants::max_thread_count> s_cullingBuckets;
        static CullingBucket s_combinedBucket;
        static bool s_combined;
    };
}

#endif