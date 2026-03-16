#include <optional>
#include <thread>

#include "tests.hpp"
#include "litl-core/thread.hpp"

namespace LITL::Core::Tests
{
    LITL_TEST_CASE("Main Thread", "[core::threadinfo]")
    {
        REQUIRE(ThreadInfo::isMainThread() == false);
        ThreadInfo::setMainThread();
        REQUIRE(ThreadInfo::isMainThread() == true);

        std::optional<bool> isOtherThreadMainThread = std::nullopt;

        std::thread otherThread([&isOtherThreadMainThread]()
            {
                isOtherThreadMainThread = ThreadInfo::isMainThread();
            });

        otherThread.join();

        REQUIRE(isOtherThreadMainThread.has_value() == true);
        REQUIRE(isOtherThreadMainThread.value() == false);
    } END_LITL_TEST_CASE
}