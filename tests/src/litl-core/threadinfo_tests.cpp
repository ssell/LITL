#include <catch2/catch_test_macros.hpp>
#include <optional>
#include <thread>

#include "litl-core/thread.hpp"

TEST_CASE("Main Thread", "[core::threadinfo]")
{
    REQUIRE(LITL::Core::ThreadInfo::isMainThread() == false);
    LITL::Core::ThreadInfo::setMainThread();
    REQUIRE(LITL::Core::ThreadInfo::isMainThread() == true);

    std::optional<bool> isOtherThreadMainThread = std::nullopt;

    std::thread otherThread([&isOtherThreadMainThread]()
        {
            isOtherThreadMainThread = LITL::Core::ThreadInfo::isMainThread();
        });

    otherThread.join();

    REQUIRE(isOtherThreadMainThread.has_value() == true);
    REQUIRE(isOtherThreadMainThread.value() == false);
}