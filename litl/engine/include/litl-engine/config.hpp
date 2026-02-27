#ifndef LITL_ENGINE_CONFIG_H__
#define LITL_ENGINE_CONFIG_H__

#include <cstdint>
#include "litl-renderer/rendererConfiguration.hpp"

namespace LITL::Engine
{
    struct SimulationConfiguration
    {
        /// <summary>
        /// The maximum number of frames rendered per second. On the range [1, 1000].
        /// </summary>
        uint32_t framesPerSecond = 120;

        /// <summary>
        /// The number of fixed update ticks per second. On the range [1, 1000].
        /// </summary>
        uint32_t ticksPerSecond = 50;
    };

    struct Configuration
    {
        SimulationConfiguration simulationSettings;
        LITL::Renderer::RendererConfiguration rendererSettings;

        void sanitize() noexcept;
    };
}

#endif