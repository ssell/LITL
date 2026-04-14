#include "tests.hpp"
#include "litl-core/types.hpp"

namespace litl::tests
{
    struct Foo
    {

    };

    struct Bar
    {

    };
}

REGISTER_TYPE_NAME(litl::tests::Foo)
REGISTER_TYPE_NAME(litl::tests::Bar)

namespace litl::tests
{
    LITL_TEST_CASE("Type IDs", "[core::types]")
    {
        const auto fooId = type_id<Foo>();
        const auto barId = type_id<Bar>();

        REQUIRE(type_id<Foo>() == fooId);
        REQUIRE(type_id<Bar>() == barId);
        REQUIRE(fooId != barId);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Type Names", "[core::types]")
    {
        const auto expectedFooName = "litl::tests::Foo";
        const auto expectedBarName = "litl::tests::Bar";

        const auto fooName = type_name<Foo>();
        const auto barName = type_name<Bar>();

        REQUIRE(type_name<Foo>() == expectedFooName);
        REQUIRE(type_name<Foo>() == fooName);

        REQUIRE(type_name<Bar>() == expectedBarName);
        REQUIRE(type_name<Bar>() == barName);
    } LITL_END_TEST_CASE
}