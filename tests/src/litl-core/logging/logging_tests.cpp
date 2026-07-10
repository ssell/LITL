
#include <array>
#include <chrono>
#include <string>
#include <thread>
#include <tuple>
#include "tests.hpp"
#include <catch2/generators/catch_generators.hpp>

#include "litl-core/logging/sinks/loggingSink.hpp"

namespace litl::tests
{
    /// <summary>
/// Supply our own test sink to capture the logged messages.
/// </summary>
    class TestLoggingSink : public LoggingSink
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
            Logger::initialize("test", false, false);
            Logger::addSink(dynamic_cast<LoggingSink*>(pSink));
        }

        ~LoggingWrapper()
        {
            Logger::shutdown();
        }

        void wait()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        TestLoggingSink* pSink;
    };

    LITL_TEST_CASE_METHOD(LoggingWrapper, "Basic Log Message", "[core::logging]")
    {
        const auto parameters = GENERATE(
            std::make_tuple<LogLevel, std::string>(LogLevel::Info, "Hello, World!"),
            std::make_tuple<LogLevel, std::string>(LogLevel::Error, "Hello, Error!")
        );

        Logger::log(std::get<LogLevel>(parameters), std::get<std::string>(parameters));
        wait();

        REQUIRE(pSink->messageBuffer.size() > 0);
        REQUIRE(pSink->messageBuffer.back().find(std::get<std::string>(parameters)) >= 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(LoggingWrapper, "Preserve Order", "[core::logging]")
    {
        const std::array<std::string, 4> parts = {
            "Hello, ",
            "World! ",
            "This is ",
            "a test."
        };

        for (const auto& part : parts)
        {
            Logger::info(part);
        }

        wait();

        REQUIRE(pSink->messageBuffer.size() == parts.size());

        for (auto i = 0; i < parts.size(); ++i)
        {
            REQUIRE(pSink->messageBuffer[i].find(parts[i]) >= 0);
        }
    } LITL_END_TEST_CASE

    namespace MultithreadedLoggingTest
    {
        void logThread(std::stop_token stopToken, uint32_t logAmount, std::string const& logMessage)
        {
            for (auto i = 0; i < logAmount; ++i)
            {
                Logger::info(logMessage);
            }
        }
    }

    LITL_TEST_CASE_METHOD(LoggingWrapper, "Multithreaded Logging", "[core::logging]")
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
    } LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(LoggingWrapper, "Parameterized Logging", "[core::logging]")
    {
        Logger::info("This message is composed of ", 3, " distinct parameters.");

        wait();

        REQUIRE(pSink->messageBuffer.size() == 1);
        REQUIRE(pSink->messageBuffer[0].find("This message is composed of 3 distinct parameters.") >= 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(LoggingWrapper, "Convenience Logging", "[core::logging]")
    {
        litl::logInfo("Hello, World!");
        litl::logInfo("Hello, ", "World!");

        wait();

        REQUIRE(pSink->messageBuffer.size() == 2);
        REQUIRE(pSink->messageBuffer[0].find("Hello, World!") >= 0);
        REQUIRE(pSink->messageBuffer[1].find("Hello, World!") >= 0);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(LoggingWrapper, "File Capture Logging", "[core::logging]")
    {
        LITL_LOG_CRITICAL_CAPTURE("Something really bad happened!");

        wait();

        REQUIRE(pSink->messageBuffer.size() == 1);
        REQUIRE(pSink->messageBuffer[0].find("Something really bad happened!") >= 0);
        REQUIRE(pSink->messageBuffer[0].find("logging_tests.cpp@") >= 0);
    } LITL_END_TEST_CASE
}