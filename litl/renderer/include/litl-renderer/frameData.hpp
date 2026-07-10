#ifndef LITL_RENDERER_FRAME_DATA_H__
#define LITL_RENDERER_FRAME_DATA_H__

#include <cstdint>

namespace litl
{
    struct FrameData
    {
        /// <summary>
        /// Total number of frames rendered
        /// </summary>
        uint32_t frameCount = 0u;

        /// <summary>
        /// The index of the current frame in flight.
        /// This is (frameCount % framesInFlight)
        /// </summary>
        uint32_t frameInFlightIndex = 0u;

        /// <summary>
        /// Number of frames in flight.
        /// </summary>
        uint32_t framesInFlight = 2u;

        void incrementFrame() noexcept
        {
            frameCount++;
            frameInFlightIndex = frameCount % framesInFlight;
        }
    };
}

#endif