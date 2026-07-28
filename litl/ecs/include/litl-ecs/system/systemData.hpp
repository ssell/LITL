#ifndef LITL_ECS_SYSTEM_DATA_H__
#define LITL_ECS_SYSTEM_DATA_H__

namespace litl
{
    class World;
    class EntityCommands;

    /// <summary>
    /// Data payload provided to the update method of each system.
    /// </summary>
    struct SystemData
    {
        /// <summary>
        /// The running ECS world.
        /// </summary>
        World& world;

        /// <summary>
        /// The command buffer for the thread.
        /// </summary>
        EntityCommands& commands;

        /// <summary>
        /// Current frame index.
        /// </summary>
        uint32_t frameIndex = 0u;

        /// <summary>
        /// Seconds since the world started running.
        /// </summary>
        float elapsedTime = 0.0f;

        /// <summary>
        /// Seconds since the last frame.
        /// </summary>
        float deltaTime = 0.0f;
    };
}

#endif