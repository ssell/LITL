#ifndef LITL_ENGINE_SCENE_NULL_PARTITION_H__
#define LITL_ENGINE_SCENE_NULL_PARTITION_H__

#include "litl-engine/scene/partition/scenePartition.hpp"

namespace litl
{
    class NullPartition
    {
    public:

        NullPartition() {}
        ~NullPartition() {}

        void add(Entity entity, bounds::AABB bounds) {}
        void remove(Entity entity) {}
        void update(Entity entity, bounds::AABB bounds) {}
        void query(bounds::AABB bounds, std::vector<Entity>& entities) {}
        void query(bounds::Sphere bounds, std::vector<Entity>& entities) {}
        void query(bounds::Frustum const& frustum, std::vector<Entity>& entities) {}

    protected:

    private:
    };

    static_assert(ScenePartition<NullPartition>);
}

#endif