#include "litl-core/math/math.hpp"
#include "litl-engine/framePacer.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// The intervals in which we spin while waiting until the next frame should start.
    /// </summary>
    constexpr auto FrameSpinThresholdNs = std::chrono::nanoseconds(200000);

    FramePacer::FramePacer()
    {
        setTargetFps(60.0f);

        // We use steady_clock in the pacer as, well, it is steady.
        // Time can move backwards in system_clock (NTP or DST), and high_resolution_clock _may_ be an alias for system_clock.
        m_nextFrameStart = std::chrono::steady_clock::now();
        m_lastFrameStart = m_nextFrameStart;
    }

    void FramePacer::setTargetFps(float fps) noexcept
    {
        fps = Math::clamp(fps, 1.0f, 1000.0f);
        m_targetPeriodNs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<float>(1.0f / fps));
    }

    void FramePacer::frameStart() noexcept
    {
        m_frameStart = std::chrono::steady_clock::now();
        m_deltaTime = std::chrono::duration<float>(m_frameStart - m_lastFrameStart).count();
    }

    void FramePacer::frameEnd() noexcept
    {
        // Target the next frame start time to prevent drifting
        m_nextFrameStart += m_targetPeriodNs;
        m_lastFrameStart = m_frameStart;

        auto now = std::chrono::steady_clock::now();

        if (now > m_nextFrameStart)
        {
            // If we took longer than our desired frame period, then there is no waiting to be done.
            // Update m_nextFrameStart so that the next frame begins immediately.
            m_nextFrameStart = now;
            return;
        }

        auto remainingTimeNs = m_nextFrameStart - now;

        // Course sleep for our fixed intervals until we have less than one whole interval remaining.
        if (remainingTimeNs > FrameSpinThresholdNs)
        {
            auto sleepTimeNs = remainingTimeNs - FrameSpinThresholdNs - m_sleepBiasNs;

            if (sleepTimeNs > std::chrono::nanoseconds::zero())
            {
                auto timeBeforeSleep = std::chrono::steady_clock::now();
                std::this_thread::sleep_for(sleepTimeNs);
                auto timeAfterSleep = std::chrono::steady_clock::now();

                // Waking from sleep is not precise and varies by OS, etc.
                // Try to adjust our sleep time using our sleep bias so we have more accurate sleep intervals.
                auto actualSleepTimeNs = timeAfterSleep - timeBeforeSleep;

                // Gradually adjust our sleep bias
                m_sleepBiasNs = (actualSleepTimeNs - sleepTimeNs) / 8;
            }
        }

        // Fine spinning for the remaining time until the next frame should start.
        while (std::chrono::steady_clock::now() < m_nextFrameStart)
        {
            std::this_thread::yield();
        }
    }

    float FramePacer::frameDelta() const noexcept
    {
        return m_deltaTime;
    }
}