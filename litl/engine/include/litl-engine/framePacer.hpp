#ifndef LITL_ENGINE_FRAME_PACER_H__
#define LITL_ENGINE_FRAME_PACER_H__

#include <chrono>
#include <thread>

namespace LITL::Engine
{
    /// <summary>
    /// Keeps the frame rate steady at the specified target FPS.
    /// </summary>
    class FramePacer
    {
    public:

        explicit FramePacer();

        void setTargetFps(float fps) noexcept;
        void frameStart() noexcept;
        void frameEnd() noexcept;

        /// <summary>
        /// Returns time (in seconds) since the 
        /// </summary>
        /// <returns></returns>
        float frameDelta() const noexcept;

    protected:

    private:

        std::chrono::steady_clock::time_point m_frameStart;
        std::chrono::steady_clock::time_point m_lastFrameStart;
        std::chrono::steady_clock::time_point m_nextFrameStart;
        float m_deltaTime;

        std::chrono::nanoseconds m_targetPeriodNs;
        std::chrono::nanoseconds m_sleepBiasNs{ 0 };
    };
}

#endif