#ifndef LITL_ENGINE_SCENE_VIEW_H__
#define LITL_ENGINE_SCENE_VIEW_H__

#include <memory>
#include <vector>

#include "litl-core/math/bounds.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    class SceneManager;
    class Scene;

    /// <summary>
    /// Provides a read-only view into the current active scene.
    /// This provides a set of operations that are safe to perform in a parallel system.
    /// </summary>
    class SceneView
    {
    public:

        SceneView();
        ~SceneView();

        /// <summary>
        /// Returns true if the entity is present in the scene.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] bool isPresent(Entity entity) const noexcept;

        /// <summary>
        /// Returns the parent of the entity. If it has no parent, returns Entity::null().
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] Entity getParent(Entity entity) const noexcept;

        /// <summary>
        /// Returns all entities that are children of the specified entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] std::vector<Entity> getChildren(Entity entity) const noexcept;

        /// <summary>
        /// Returns the index into the GPU buffers which the entity occupies.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getGpuBufferIndex(Entity entity) const noexcept;

        /// <summary>
        /// Returns all entities that are within or intersect the specified AABB.
        /// </summary>
        /// <param name="aabb"></param>
        /// <param name="entities"></param>
        void query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept;

        /// <summary>
        /// Returns all entities that are within or intersect the specified Sphere.
        /// </summary>
        /// <param name="sphere"></param>
        /// <param name="entities"></param>
        void query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept;

        /// <summary>
        /// Returns all entities that are within or intersect the specified Frustum.
        /// </summary>
        /// <param name="frustum"></param>
        /// <param name="entities"></param>
        void query(bounds::Frustum frustum, std::vector<Entity>& entities) const noexcept;

    protected:

    private:

        friend class SceneManager;
        void setViewedScene(std::shared_ptr<Scene> scene) noexcept;

        std::shared_ptr<Scene> m_pActiveScene;
    };
}

#endif