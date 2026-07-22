#ifndef LITL_ENGINE_SCENE_VIEW_H__
#define LITL_ENGINE_SCENE_VIEW_H__

#include <memory>
#include <span>
#include <vector>

#include "litl-core/math/bounds.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class SceneManager;
    class Scene;
    class Camera;
    struct Transform;

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
        /// Adds the entity to the scene.
        /// This is not needed if the entity is made via EntityCommands.
        /// 
        /// Note that this method is not thread-safe and should only be invoked from
        /// the main thread at a sync point.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="transform"></param>
        void track(Entity entity, Transform const& transform) noexcept;

        /// <summary>
        /// Adds the entity to the scene.
        /// This is not needed if the entity is made via EntityCommands.
        /// 
        /// Note that this method is not thread-safe and should only be invoked from
        /// the main thread at a sync point.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="transform"></param>
        /// <param name="bounds"></param>
        void track(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept;

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
        /// Retrieves the previously calculated world matrix for the specified entity.
        /// World matrices are calculated once per frame immediately prior the PreRender ECS grouping.
        /// As such the matrix may be up to one frame stale.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] mat4 getWorldMatrix(Entity entity) const noexcept;

        /// <summary>
        /// Returns a read-only span of all entity world matrices.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::span<mat4 const> getWorldMatrices() const noexcept;

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

        /// <summary>
        /// Sets the specified valid camera handle as the main camera.
        /// The main camera is the one used to render to the primary swapchain render target.
        /// </summary>
        /// <param name="handle"></param>
        void setMainCamera(CameraHandle handle) const noexcept;

        /// <summary>
        /// Returns the handle of the camera that was last designated as the main camera.
        /// If no main camera was set, or it has been destroyed, this may return an invalid handle.
        /// </summary>
        /// <returns></returns>
        CameraHandle getMainCameraHandle() const noexcept;

        /// <summary>
        /// Returns the camera that was last designated as the main camera.
        /// If no main camera was set, or it has been destroyed, this may return null.
        /// </summary>
        /// <returns></returns>
        Camera* getMainCamera() const noexcept;

        /// <summary>
        /// Returns all cameras. They are sorted according to their process order value.
        /// </summary>
        /// <returns></returns>
        std::span<Camera*> getCameras() noexcept;

    protected:

    private:

        friend class SceneManager;
        void setViewedScene(std::shared_ptr<Scene> scene) noexcept;

        std::shared_ptr<Scene> m_pActiveScene;
    };
}

#endif