

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <tuple>

#include "litl-core/logging/logLevel.hpp"
#define LITL_LOG_LEVEL LITL_LOG_LEVEL_INFO
// ^ intentionally hide debug/trace logs for testing purposes
#include "litl-core/logging/logging.hpp"
#include "litl-core/logging/sinks/loggingSink.hpp"

/// <summary>
/// Supply our own test sink to capture the logged message.
/// </summary>
class TestLoggingSink : public LITL::Core::LoggingSink
{
public:

    std::optional<std::string> lastMessage = std::nullopt;

protected:

    void processMessage(std::string const& message) override
    {
        lastMessage = message;
    }

private:
};

/// <summary>
/// Handles initializing the logger for the tests.
/// </summary>
class LoggingWrapper
{
public:

    LoggingWrapper()
        : pSink(new TestLoggingSink())
    {
        LITL::Core::Logger::initialize("test", false, false);
        LITL::Core::Logger::addSink(dynamic_cast<LITL::Core::LoggingSink*>(pSink));
    }

    ~LoggingWrapper()
    {
        LITL::Core::Logger::shutdown();
    }

    TestLoggingSink* pSink;
};

TEST_CASE_METHOD(LoggingWrapper, "Basic Log Message", "[core::logging]")
{
    const auto parameters = GENERATE(
        std::make_tuple<LITL::Core::LogLevel, std::string>(LITL::Core::LogLevel::Info, "Hello, World!"),
        std::make_tuple<LITL::Core::LogLevel, std::string>(LITL::Core::LogLevel::Error, "Hello, Error!")
    );

    LITL::Core::Logger::log(std::get<LITL::Core::LogLevel>(parameters), std::get<std::string>(parameters));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    REQUIRE(pSink->lastMessage == std::get<std::string>(parameters));
}

TEST_CASE_METHOD(LoggingWrapper, "Filter Log Message", "[core::logging]")
{
    // See: "#define LITL_LOG_LEVEL LITL_LOG_LEVEL_INFO" at top of this file
    const auto parameters = GENERATE(
        std::make_tuple(LITL::Core::LogLevel::Trace, false),
        std::make_tuple(LITL::Core::LogLevel::Debug, false),
        std::make_tuple(LITL::Core::LogLevel::Info, true),
        std::make_tuple(LITL::Core::LogLevel::Warning, true)
    );

    LITL::Core::Logger::log(std::get<LITL::Core::LogLevel>(parameters), "Hello, World!");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    REQUIRE((pSink->lastMessage == std::nullopt) == !std::get<bool>(parameters));
}