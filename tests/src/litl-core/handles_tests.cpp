#include "tests.hpp"
#include "litl-core/handles.hpp"

namespace litl::tests
{
    struct TestData
    {
        uint32_t data;
    };

    struct TestTag {};
    using TestHandle = litl::Handle<TestTag>;

    LITL_TEST_CASE("HandlePool", "[core::handles]")
    {
        HandlePool<TestData, TestTag> pool;

        TestData data0{ 0 };
        TestData data1{ 1 };
        TestData data2{ 2 };

        auto handle0 = pool.create(data0);
        auto handle1 = pool.create(data1);
        auto handle2 = pool.create(data2);

        REQUIRE(handle0.index == 0);
        REQUIRE(handle0.version == 1);

        REQUIRE(handle1.index == 1);
        REQUIRE(handle1.version == 1);

        REQUIRE(handle2.index == 2);
        REQUIRE(handle2.version == 1);

        REQUIRE((*pool.get(handle0)).data == 0);
        REQUIRE((*pool.get(handle1)).data == 1);
        REQUIRE((*pool.get(handle2)).data == 2);

        REQUIRE(pool.valid(handle1) == true);
        pool.destroy(handle1);
        REQUIRE(pool.valid(handle1) == false);

        REQUIRE((*pool.get(handle0)).data == 0);
        REQUIRE(pool.get(handle1) == nullptr);
        REQUIRE((*pool.get(handle2)).data == 2);

        pool.destroy(handle1);      // no operation

        REQUIRE((*pool.get(handle0)).data == 0);
        REQUIRE(pool.get(handle1) == nullptr);
        REQUIRE((*pool.get(handle2)).data == 2);

        handle1 = pool.create(data1);

        REQUIRE(handle1.index == 1);    // reuse
        REQUIRE(handle1.version == 3);  // delete increments as does create, so +2 from last time
        REQUIRE((*pool.get(handle1)).data == 1);
    } LITL_END_TEST_CASE
}
