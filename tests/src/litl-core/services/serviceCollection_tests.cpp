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

    class FooService
    {
    public:

        FooService(uint32_t value) : m_value(value) {}
        uint32_t get() const noexcept { return m_value; }

    private:

        uint32_t m_value;
    };
}

namespace LITL::Core::Tests
{
    TEST_CASE("Add", "[core::services]")
    {
        ServiceCollection services;
        REQUIRE(services.size() == 0);

        services.add<IValueService, DoublingService>(ServiceLifetime::Singleton, [](auto resolver)
            {
                return std::make_shared<DoublingService>();
            });

        REQUIRE(services.size() == 1);
    }

    TEST_CASE("Add Singleton", "[core::services]")
    {
        ServiceCollection services;
        REQUIRE(services.size() == 0);

        services.addSingleton<IValueService, DoublingService>();

        REQUIRE(services.size() == 1);

        services.addSingleton<FooService>([](auto resolver)
            {
                return std::make_shared<FooService>(55);
            });

        REQUIRE(services.size() == 2);
    }

    TEST_CASE("Get Missing", "[core::services]")
    {
        ServiceCollection services;
        auto provider = services.build();

        REQUIRE(provider->get<FooService>() == nullptr);
    }

    TEST_CASE("Get Singleton", "[core::services]")
    {
        ServiceCollection services;
        services.addSingleton<IValueService, DoublingService>();
        auto provider = services.build();

        auto doubler = provider->get<IValueService>();
        doubler->set(1);

        REQUIRE(doubler->get() == 2);
    }

    TEST_CASE("Set Singleton Multiple Times", "[core::services]")
    {
        ServiceCollection services;
        services.addSingleton<IValueService, DoublingService>();
        services.addSingleton<IValueService, TriplingService>();
        auto provider = services.build();

        // should return the last type set, so a TriplingService.
        auto tripler = provider->get<IValueService>();
        tripler->set(1);

        REQUIRE(tripler->get() == 3);
    }

    TEST_CASE("Multiple Singletons", "[core::services]")
    {
        ServiceCollection services;
        services.addSingleton<IValueService, DoublingService>();
        services.addSingleton<FooService>([](auto resolver) { return std::make_shared<FooService>(55); });
        auto provider = services.build();

        auto fooService = provider->get<FooService>();
        auto valueService = provider->get<IValueService>();
        valueService->set(10);

        REQUIRE(valueService->get() == 20);
        REQUIRE(fooService->get() == 55);
    }
}