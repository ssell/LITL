#include <any>
#include <catch2/catch_test_macros.hpp>

#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace
{
    class IValueService
    {
    public:

        virtual ~IValueService() = default;
        void set(uint32_t x) { value = x; }
        virtual uint32_t get() const noexcept { return value; }

    protected:

        uint32_t value{ 0 };
    };

    class DoublingService : public IValueService
    {
    public:

        uint32_t get() const noexcept override { return value * 2; }
    };

    class TriplingService : public IValueService
    {
    public:

        uint32_t get() const noexcept override { return value * 3; }
    };

    class IFooService
    {
    public:

        IFooService(IValueService* testService) : service(testService) {}
        ~IFooService() = default;
        IValueService* service;
    };

    class FooService : public IFooService
    {
    public:

        FooService(IValueService* testService) : IFooService(testService) {}
        ~FooService() = default;
    };
}

TEST_CASE("Add", "[core::serviceCollection]")
{
    LITL::Core::ServiceCollection services;
    REQUIRE(services.size() == 0);

    services.add<IValueService, DoublingService>(LITL::Core::ServiceLifetime::Singleton, [](auto resolver)
        {
            return std::make_shared<DoublingService>();
        });

    REQUIRE(services.size() == 1);
}

TEST_CASE("Add Singleton", "[core::serviceCollection]")
{
    LITL::Core::ServiceCollection services;
    REQUIRE(services.size() == 0);

    services.addSingleton<IValueService, DoublingService>();

    REQUIRE(services.size() == 1);

    services.addSingleton<IFooService, FooService>([](auto resolver)
        {
            auto testService = LITL::Core::resolveService<IValueService>(resolver);
            return std::make_shared<FooService>(testService.get());
        });

    REQUIRE(services.size() == 2);
}

TEST_CASE("Get Singleton", "[core::serviceCollection]")
{
    LITL::Core::ServiceCollection services;
    services.addSingleton<IValueService, DoublingService>();
    auto provider = services.build();

    auto doubler = provider->get<IValueService>();
    doubler->set(1);

    REQUIRE(doubler->get() == 2);
}

TEST_CASE("Set Singleton Multiple Times", "[core::serviceCollection]")
{
    LITL::Core::ServiceCollection services;
    services.addSingleton<IValueService, DoublingService>();
    services.addSingleton<IValueService, TriplingService>();
    auto provider = services.build();

    // should return the last type set, so a TriplingService.
    auto tripler = provider->get<IValueService>();
    tripler->set(1);

    REQUIRE(tripler->get() == 3);
}