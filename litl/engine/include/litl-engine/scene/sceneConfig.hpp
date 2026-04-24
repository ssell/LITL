#ifndef LITL_ENGINE_SCENE_CONFIG_H__
#define LITL_ENGINE_SCENE_CONFIG_H__

#include "litl-engine/scene/partition/scenePartition.hpp"
#include "litl-engine/scene/partition/partitionOptions.hpp"

namespace litl
{
    struct SceneConfig
    {
        ScenePartitionType partition{ ScenePartitionType::UniformGrid };
        UniformGridOptions uniformGridOptions{};
    };
}

#endif