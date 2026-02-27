#include <catch2/catch_test_macros.hpp>
#include "litl-engine/framePacer.hpp"

TEST_CASE("Frame Pacer - Frame Loop", "[engine::frame]")
{
    LITL::Engine::FramePacer framePacer;
    framePacer.setTargetFps(60.0f);

    uint32_t frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();

    while (frameCount < 60)
    {
        framePacer.frameStart();
        framePacer.frameEnd();
        frameCount++;
    }

    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = endTime - startTime;

    // Expect one second to have passed (1,000,000,000 nanoseconds)
}