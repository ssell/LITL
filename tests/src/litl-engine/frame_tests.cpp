#include <catch2/catch_test_macros.hpp>
#include "litl-engine/frameLimiter.hpp"

namespace
{
    constexpr uint64_t nsInSec = static_cast<uint64_t>(1000000000ul);
}

TEST_CASE("Frame Limiter 60 FPS", "[engine::frame]")
{
    LITL::Engine::FrameLimiter frameLimiter;
    frameLimiter.setTargetFps(60.0f);

    uint32_t frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();

    while (frameCount < 60)
    {
        frameLimiter.frameStart();
        frameLimiter.frameEnd();
        frameCount++;
    }

    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = static_cast<uint64_t>((endTime - startTime).count());


    // 60 frames should have taken AT LEAST 1 seconds due to the frame limiter.
    // It is difficult/unreliable to measure at a finer level due to OS variability in granularity for sleep_for (which runs the coarse sleep section)
    REQUIRE(elapsed > nsInSec);
    REQUIRE(elapsed < (nsInSec * 2));
}

TEST_CASE("Frame Limiter 30 FPS", "[engine::frame]")
{
    LITL::Engine::FrameLimiter frameLimiter;
    frameLimiter.setTargetFps(30.0f);

    uint32_t frameCount = 0;
    auto startTime = std::chrono::steady_clock::now();

    while (frameCount < 60)
    {
        frameLimiter.frameStart();
        frameLimiter.frameEnd();
        frameCount++;
    }

    auto endTime = std::chrono::steady_clock::now();
    auto elapsed = static_cast<uint64_t>((endTime - startTime).count());

    // 60 frames should have taken AT LEAST 2 seconds due to the frame limiter.
    REQUIRE(elapsed > (nsInSec * 2));
    REQUIRE(elapsed < (nsInSec * 3));
}