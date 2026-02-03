
#include <array>
#include <chrono>
#include <string>
#include <thread>
#include <tuple>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "litl-core/logging/logLevel.hpp"
#define LITL_LOG_LEVEL LITL_LOG_LEVEL_INFO
// ^ intentionally hide debug/trace logs for testing purposes
#include "litl-core/logging/logging.hpp"
#include "litl-core/logging/sinks/loggingSink.hpp"

/// <summary>
/// Supply our own test sink to capture the logged messages.
/// </summary>
class TestLoggingSink : public LITL::Core::LoggingSink
{
public:

    std::vector<std::string> messageBuffer;

protected:

    void processMessage(std::string const& message) override
    {
        messageBuffer.push_back(message);
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

    void wait()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
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
    wait();
    
    REQUIRE(pSink->messageBuffer.size() > 0);
    REQUIRE(pSink->messageBuffer.back() == std::get<std::string>(parameters));
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
    wait();

    if (std::get<bool>(parameters))
    {
        REQUIRE(pSink->messageBuffer.size() > 0);
    }
    else
    {
        REQUIRE(pSink->messageBuffer.size() == 0);
    }
}

TEST_CASE_METHOD(LoggingWrapper, "Preserve Order", "[core::logging]")
{
    const std::array<std::string, 4> parts = {
        "Hello, ",
        "World! ",
        "This is ",
        "a test."
    };

    for (const auto& part : parts)
    {
        LITL::Core::Logger::info(part);
    }

    wait();

    REQUIRE(pSink->messageBuffer.size() == parts.size());

    for (auto i = 0; i < parts.size(); ++i)
    {
        REQUIRE(pSink->messageBuffer[i] == parts[i]);
    }
}

namespace MultithreadedLoggingTest
{
    void logThread(std::stop_token stopToken, uint32_t logAmount, std::string const& logMessage)
    {
        for (auto i = 0; i < logAmount; ++i)
        {
            LITL::Core::Logger::info(logMessage);
        }
    }
}

TEST_CASE_METHOD(LoggingWrapper, "Multithreaded Logging", "[core::logging]")
{
    std::jthread thread0(MultithreadedLoggingTest::logThread, 10, "Thread0");
    std::jthread thread1(MultithreadedLoggingTest::logThread, 10, "Thread1");
    std::jthread thread2(MultithreadedLoggingTest::logThread, 10, "Thread2");

    int iterations = 10;

    while ((pSink->messageBuffer.size() < 30) && (iterations > 0))
    {
        wait();
        iterations--;
    }

    REQUIRE(pSink->messageBuffer.size() == 30);
}

TEST_CASE_METHOD(LoggingWrapper, "Parameterized Logging", "[core::logging]")
{
    LITL::Core::Logger::info("This message is composed of ", 3, " distinct parameters.");

    wait();

    REQUIRE(pSink->messageBuffer.size() == 1);
    REQUIRE(pSink->messageBuffer[0] == "This message is composed of 3 distinct parameters.");
}

TEST_CASE_METHOD(LoggingWrapper, "Convenience Logging", "[core::logging]")
{
    LITL::logInfo("Hello, World!");
    LITL::logInfo("Hello, ", "World!");

    wait();

    REQUIRE(pSink->messageBuffer.size() == 2);
    REQUIRE(pSink->messageBuffer[0] == "Hello, World!");
    REQUIRE(pSink->messageBuffer[1] == "Hello, World!");
}

TEST_CASE_METHOD(LoggingWrapper, "File Capture Logging", "[core::logging]")
{
    LITL_LOG_CRITICAL_CAPTURE("Something really bad happened!");

    wait();

    REQUIRE(pSink->messageBuffer.size() == 1);
    REQUIRE(pSink->messageBuffer[0].find("Something really bad happened!") >= 0);
    REQUIRE(pSink->messageBuffer[0].find("logging_tests.cpp@") >= 0);
}