#include <variant>

#include "litl-core/assert.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/scenegraph.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"
#include "litl-engine/scene/partition/nullPartition.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl
{
    using ScenePartitionVariant = std::variant<
        NullPartition,
        UniformGridPartition
        /* add future partition strategies here */
    >;

    struct Scene::Impl
    {
        SceneGraph graph;
        ScenePartitionVariant partition;
    };

    Scene::Scene(SceneConfig const& config)
    {
        switch (config.partition)
        {
        case ScenePartitionType::UniformGrid:
            m_impl->partition.emplace<UniformGridPartition>(config.uniformGridOptions);
            break;

        default:
            LITL_ASSERT_MSG(false, "Unsupported Scene Partition strategy.", );
        }
    }

    Scene::~Scene()
    {

    }
}