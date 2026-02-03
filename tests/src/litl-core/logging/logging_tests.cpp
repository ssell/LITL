#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <chrono>
#include <optional>
#include <string_view>
#include <thread>

#include "litl-core/logging/logging.hpp"
#include "litl-core/logging/sinks/loggingSink.hpp"

/// <summary>
/// Supply our own test sink to capture the logged message.
/// </summary>
class TestLoggingSink : public LITL::Core::LoggingSink
{
public:

    std::optional<std::string_view> lastMessage = std::nullopt;

protected:

    void processMessage(std::string_view message) override
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
    const LITL::Core::LogLevel logLevel = GENERATE(
        LITL::Core::LogLevel::Info, 
        LITL::Core::LogLevel::Error);

    const std::string_view message = GENERATE(
        "Hello, World!", 
        "Hello, Error!");

    LITL::Core::Logger::log(logLevel, message);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    
    REQUIRE(pSink->lastMessage == message);
}