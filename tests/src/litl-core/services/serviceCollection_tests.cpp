#include <any>
#include <catch2/catch_test_macros.hpp>

#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace
{
    class ITestService
    {
    public:

        uint32_t count{ 0 };
    };

    class TestService : public ITestService
    {

    };

    class IFooService
    {
    public:

        IFooService(ITestService* testService) : service(testService) {}
        ~IFooService() = default;
        ITestService* service;
    };

    class FooService : public IFooService
    {
    public:

        FooService(ITestService* testService) : IFooService(testService) {}
        ~FooService() = default;
    };
}

TEST_CASE("Add", "[core::serviceCollection]")
{
    LITL::Core::ServiceCollection services;
    REQUIRE(services.size() == 0);

    services.add<ITestService, TestService>(LITL::Core::ServiceLifetime::Singleton, [](auto resolver)
        {
            return std::make_shared<TestService>();
        });

    REQUIRE(services.size() == 1);
}

TEST_CASE("Add Singleton", "[core::serviceCollection]")
{
    LITL::Core::ServiceCollection services;
    REQUIRE(services.size() == 0);

    services.addSingleton<ITestService, TestService>();

    REQUIRE(services.size() == 1);

    services.addSingleton<IFooService, FooService>([](auto resolver)
        {
            auto testService = LITL::Core::resolveService<ITestService>(resolver);
            return std::make_shared<FooService>(testService.get());
        });

    REQUIRE(services.size() == 2);
}

TEST_CASE("Get Singleton", "[core::serviceCollection]")
{
    LITL::Core::ServiceCollection services;
    services.addSingleton<ITestService, TestService>();

    auto provider = services.build();
    auto testService0 = provider->get<ITestService>();
    auto testService1 = provider->get<ITestService>();

    REQUIRE(testService0 != nullptr);
    REQUIRE(testService1 != nullptr);
    REQUIRE(testService0.get() == testService1.get());

    testService0->count++;
    testService1->count++;

    REQUIRE(testService0->count == 2);
}