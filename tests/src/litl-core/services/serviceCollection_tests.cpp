#include <any>

#include "tests.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/services/serviceScope.hpp"

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

namespace litl::tests
{
    LITL_TEST_CASE("Add", "[core::services]")
    {
        ServiceCollection services;
        REQUIRE(services.size() == 0);

        services.add<IValueService, DoublingService>(ServiceLifetime::Singleton, [](auto)
            {
                return std::make_shared<DoublingService>();
            });

        REQUIRE(services.size() == 1);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Add Singleton", "[core::services]")
    {
        ServiceCollection services;
        REQUIRE(services.size() == 0);

        services.addSingleton<IValueService, DoublingService>();

        REQUIRE(services.size() == 1);

        services.addSingleton<FooService>([](auto)
            {
                return std::make_shared<FooService>(55);
            });

        REQUIRE(services.size() == 2);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Get Missing", "[core::services]")
    {
        ServiceCollection services;
        auto provider = services.build();

        REQUIRE(provider->get<FooService>() == nullptr);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Get Singleton", "[core::services]")
    {
        ServiceCollection services;
        services.addSingleton<IValueService, DoublingService>();
        auto provider = services.build();

        auto doubler = provider->get<IValueService>();
        doubler->set(1);

        REQUIRE(doubler->get() == 2);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Set Singleton Multiple Times", "[core::services]")
    {
        ServiceCollection services;
        services.addSingleton<IValueService, DoublingService>();
        services.addSingleton<IValueService, TriplingService>();
        auto provider = services.build();

        // should return the last type set, so a TriplingService.
        auto tripler = provider->get<IValueService>();
        tripler->set(1);

        REQUIRE(tripler->get() == 3);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Multiple Singletons", "[core::services]")
    {
        ServiceCollection services;
        services.addSingleton<IValueService, DoublingService>();
        services.addSingleton<FooService>([](auto) { return std::make_shared<FooService>(55); });
        auto provider = services.build();

        auto fooService = provider->get<FooService>();
        auto valueService = provider->get<IValueService>();
        valueService->set(10);

        REQUIRE(valueService->get() == 20);
        REQUIRE(fooService->get() == 55);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Scoped Service", "[core::services]")
    {
        ServiceCollection services;
        services.addScoped<IValueService, DoublingService>();
        auto provider = services.build();
        auto scope = provider->createScope();

        auto nonScopedService = provider->get<IValueService>();
        REQUIRE(nonScopedService == nullptr);

        auto scopedService = scope->get<IValueService>();
        REQUIRE(scopedService != nullptr);

        scopedService->set(5);
        REQUIRE(scopedService->get() == 10);

        auto scopedService2 = scope->get<IValueService>();
        REQUIRE(scopedService.get() == scopedService2.get());
        REQUIRE(scopedService2->get() == 10);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Scoped Singleton Service", "[core::services]")
    {
        ServiceCollection services;
        services.addSingleton<FooService>([](auto) { return std::make_shared<FooService>(55); });
        auto provider = services.build();
        auto scope = provider->createScope();

        auto providerSingleton = provider->get<FooService>();
        auto scopeSingleton = provider->get<FooService>();

        REQUIRE((providerSingleton.get()) == (scopeSingleton.get()));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Transient Service", "[core::services]")
    {
        ServiceCollection services;
        services.addTransient<IValueService, DoublingService>();
        auto provider = services.build();
        auto scope = provider->createScope();

        auto transient0 = provider->get<IValueService>();
        auto transient1 = provider->get<IValueService>();
        auto transient2 = scope->get<IValueService>();
        auto transient3 = scope->get<IValueService>();

        REQUIRE(transient0.get() != transient1.get());
        REQUIRE(transient0.get() != transient2.get());
        REQUIRE(transient0.get() != transient3.get());
        REQUIRE(transient1.get() != transient2.get());
        REQUIRE(transient2.get() != transient3.get());

        transient0->set(5);
        transient1->set(10);
        transient2->set(15);
        transient3->set(20);

        REQUIRE(transient0->get() == 10);
        REQUIRE(transient1->get() == 20);
        REQUIRE(transient2->get() == 30);
        REQUIRE(transient3->get() == 40);
    } END_LITL_TEST_CASE
}