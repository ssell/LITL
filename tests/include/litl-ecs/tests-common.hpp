#ifndef LITL_TESTS_ECS_COMMON_H__
#define LITL_TESTS_ECS_COMMON_H__

#include "litl-core/types.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/math.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/constants.hpp"

namespace litl::tests
{
    struct Foo
    {
        uint32_t a{ 0 };
    };

    struct Bar
    {
        float a{ 0.0f };
        uint32_t b{ 0 };
    };

    struct Baz
    {
        bool ok{ false };
    };

    struct SystemSetupService
    {
        bool wasSetup{ false };
    };

    static bool& getTestSystemPrepared() noexcept
    {
        static bool prepared = false;
        return prepared;
    }

    static void setTestSystemPrepared(bool prepared)
    {
        getTestSystemPrepared() = prepared;
    }

    struct TestSystem
    {
        void setup(ServiceProvider& services)
        {
            setTestSystemPrepared(false);

            auto setupService = services.get<SystemSetupService>();

            if (setupService != nullptr)
            {
                setupService->wasSetup = true;
            }
        }

        void prepare()
        {
            setTestSystemPrepared(true);
        }

        void update(EntityCommands& commands, float dt, Entity entity, Foo& foo, Bar& bar)
        {
            foo.a++;
            bar.b++;
        }
    };
}

LITL_REGISTER_TYPE_NAME(litl::tests::Foo)
LITL_REGISTER_TYPE_NAME(litl::tests::Bar)
LITL_REGISTER_TYPE_NAME(litl::tests::Baz)

#endif