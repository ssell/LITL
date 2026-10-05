#ifndef LITL_SAMPLES_BUNNY_FLASH_H__
#define LITL_SAMPLES_BUNNY_FLASH_H__

#include <memory>

#include "litl-core/math/common.hpp"
#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl::samples
{
    struct Flash
    {
        float rate{ 1.0f };
    };

    struct FlashSystem final
    {
        void setup(ServiceProvider& services) 
        {
            m_pObjectPool = services.get<ObjectPool>();
        }

        void prepare() {}

        void update(SystemData const& data, Entity entity, Flash const& flash, MaterialRef& materialRef)
        {
            auto* material = m_pObjectPool->getMaterial(materialRef.handle);

            if (material == nullptr)
            {
                return;
            }

            material->setFloat("fade"_sid, (sin(data.elapsedTime * flash.rate) + 1.0f) * 0.5f, materialRef.slot);
        }

    private:

        std::shared_ptr<ObjectPool> m_pObjectPool;
    };
}

LITL_REGISTER_COMPONENT(litl::samples::Flash);

#endif