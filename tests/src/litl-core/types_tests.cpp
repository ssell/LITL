#include "tests.hpp"
#include "litl-core/types.hpp"

namespace LITL::Core::Tests
{
    struct Foo
    {

    };

    struct Bar
    {

    };
}

REGISTER_TYPE_NAME(LITL::Core::Tests::Foo)
REGISTER_TYPE_NAME(LITL::Core::Tests::Bar)

namespace LITL::Core::Tests
{
    LITL_TEST_CASE("Type IDs", "[core::types]")
    {
        const auto fooId = type_id<Foo>();
        const auto barId = type_id<Bar>();

        REQUIRE(type_id<Foo>() == fooId);
        REQUIRE(type_id<Bar>() == barId);
        REQUIRE(fooId != barId);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("Type Names", "[core::types]")
    {
        const auto expectedFooName = "LITL::Core::Tests::Foo";
        const auto expectedBarName = "LITL::Core::Tests::Bar";

        const auto fooName = type_name<Foo>();
        const auto barName = type_name<Bar>();

        REQUIRE(type_name<Foo>() == expectedFooName);
        REQUIRE(type_name<Foo>() == fooName);

        REQUIRE(type_name<Bar>() == expectedBarName);
        REQUIRE(type_name<Bar>() == barName);
    } END_LITL_TEST_CASE
}