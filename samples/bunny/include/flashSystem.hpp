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

            if (!materialRef.frequentUpdates)
            {
                // This could be set at material slot allocation, which may be practical for some use cases but not for others.
                // For example, if you have a bomb entity and they only need to start flashing at a certain time then it is a waste to
                // have them marked as frequently updating before they need to start flashing.
                materialRef.frequentUpdates = material->markAsFrequentUpdate(materialRef.slot, false);
                // todo: a way to know when to set a material slot as no longer needing frequent updates.
                // perhaps an expiration frame count in markAsFrequentUpdate that needs to be refreshed,
                // or detection within MaterialProperties itself if a "frequently updating" slot hasn't actually
                // had new data set in a number of frames and having the designation fall off.
            }

            if (material != nullptr)
            {
                material->setFloat("fade"_sid, (sin(data.elapsedTime * flash.rate) + 1.0f) * 0.5f, materialRef.slot);
            }
        }

    private:

        std::shared_ptr<ObjectPool> m_pObjectPool;
    };
}

LITL_REGISTER_COMPONENT(litl::samples::Flash);

#endif