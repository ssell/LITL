#ifndef LITL_ENGINE_ECS_LOADING_ASSET_SYSTEM_H__
#define LITL_ENGINE_ECS_LOADING_ASSET_SYSTEM_H__

#include <memory>

#include "litl-ecs/system/systemData.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/loadingAsset.hpp"

namespace litl
{
    class AssetManager;
    class ServiceProvider;

    class LoadingAssetSystem final
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, LoadingAsset& loading);

    private:

        std::shared_ptr<AssetManager> m_pAssetManager{ nullptr };
    };
}

#endif