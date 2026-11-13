#ifndef LITL_ENGINE_LIGHTS_SHADOW_ALLOCATION_H__
#define LITL_ENGINE_LIGHTS_SHADOW_ALLOCATION_H__

#include "litl-engine/lights/shadowCasterType.hpp"

namespace litl
{
    struct ShadowAllocation
    {
        /// <summary>
        /// The type of shadow caster that the allocation is for.
        /// </summary>
        ShadowCasterType type;

        /// <summary>
        /// First layer into the shadow caster type's array texture.
        /// </summary>
        uint32_t firstLayer;

        /// <summary>
        /// Shadowmap layers, dependent on shadow caster type.
        /// Directional = # of cascades, Spot = 1, Point = 6 (cubemap)
        /// </summary>
        uint32_t layerCount;
    };

    class ShadowAllocator
    {
    public:

        

    private:


    };
}

#endif