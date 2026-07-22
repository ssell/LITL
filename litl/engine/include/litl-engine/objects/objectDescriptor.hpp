#ifndef LITL_ENGINE_OBJECT_DESCRIPTOR_H__
#define LITL_ENGINE_OBJECT_DESCRIPTOR_H__

#include <cstdint>

namespace litl
{
    enum class ObjectLifetime : uint32_t
    {
        /// <summary>
        /// Object lives for the lifetime of the application and is not removed until engine shutdown.
        /// </summary>
        Application = 0u,

        /// <summary>
        /// Object lives for the lifetime of the current scene and is removed when the scene switches.
        /// todo: not yet implemented.
        /// </summary>
        Scene = 1u,

        /// <summary>
        /// Object lives until it is not active for a given period of time.
        /// This time period is set in the engine configuration.
        /// todo: not yet implemented.
        /// </summary>
        ActiveTime = 2u,

        /// <summary>
        /// Object lives until it is not active for a given number of frames.
        /// This frame count is set in the engine configuration.
        /// todo: not yet implemented.
        /// </summary>
        ActiveFrames = 3u,

        /// <summary>
        /// Object lives until there are no references remaining.
        /// todo: not yet implemented.
        /// </summary>
        ActiveReferences = 4u
    };

    struct ObjectDescriptor
    {
        /// <summary>
        /// Optional name. Used for debugging and diagnostics.
        /// </summary>
        std::string name;

        /// <summary>
        /// How long the object persists for.
        /// </summary>
        ObjectLifetime lifetime = ObjectLifetime::Application;
    };
}

#endif