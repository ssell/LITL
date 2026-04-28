#ifndef LITL_ENGINE_SCENE_NULL_PARTITION_H__
#define LITL_ENGINE_SCENE_NULL_PARTITION_H__

#include "litl-engine/scene/partition/scenePartition.hpp"

namespace litl
{
    /// <summary>
    /// A partition that does nothing.
    /// </summary>
    class NullPartition
    {
    public:

        NullPartition() {}
        ~NullPartition() {}

        void add(Entity entity, bounds::AABB bounds) noexcept {}
        void remove(Entity entity) noexcept {}
        void update(Entity entity, bounds::AABB bounds) noexcept {}
        void query(bounds::AABB bounds, std::vector<Entity>& entities) const noexcept {}
        void query(bounds::Sphere bounds, std::vector<Entity>& entities) const noexcept {}
        void query(bounds::Frustum const& frustum, std::vector<Entity>& entities) const noexcept {}

    protected:

    private:
    };

    static_assert(ScenePartition<NullPartition>);
}

#endif