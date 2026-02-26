#ifndef LITL_ENGINE_ECS_SYSTEM_GROUP_H__
#define LITL_ENGINE_ECS_SYSTEM_GROUP_H__

#include <cstdint>

namespace LITL::ECS
{
    enum class SystemGroup : uint32_t
    {
        /// <summary>
        /// First group of systems to run each frame.
        /// </summary>
        Startup = 0,

        /// <summary>
        /// Systems which interact with IO.
        /// </summary>
        Input = 1,

        /// <summary>
        /// Systems which run at a fixed step regardless of frame rate.
        /// These may run multiple times per render frame.
        /// </summary>
        FixedUpdate = 2,

        /// <summary>
        /// Systems which are run once-per render frame.
        /// </summary>
        Update = 3,

        /// <summary>
        /// Systems run once-per frame following the standard Update group.
        /// </summary>
        LateUpdate = 4,

        /// <summary>
        /// Systems run once-per frame after all update groups and immediately
        /// prior to the render extraction systems.
        /// </summary>
        PreRender = 5,

        /// <summary>
        /// Systems run once-per frame to interact with the rendering subsystem.
        /// </summary>
        Render = 6,

        /// <summary>
        /// Systems run once-per frame following rendering.
        /// </summary>
        PostRender = 7,

        /// <summary>
        /// The last group of systems to run each frame.
        /// </summary>
        Final = 8
    };

    constexpr uint32_t SystemGroupCount = 9;
}

#endif