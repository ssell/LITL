#include "tests.hpp"
#include "litl-core/containers/memoryPool.hpp"

namespace LITL::Core::Tests
{
    namespace
    {
        struct TestStructA
        {
            uint32_t a{ 0 };
        };

        struct TestStructB
        {
            uint32_t a{ 0 };
            uint32_t b{ 0 };
        };
    }
    LITL_TEST_CASE("Basic", "[core::containers::memoryPool]")
    {
        MemoryPool<> pool;

        TestStructA a{ 55 };
        TestStructB b{ 100, 33 };

        void* aDest = pool.copyInto(&a, sizeof(TestStructA));
        void* bDest = pool.copyInto(&b, sizeof(TestStructB));

        TestStructA aExtracted{};
        TestStructB bExtracted{};

        REQUIRE(aExtracted.a != a.a);
        REQUIRE(bExtracted.a != b.a);
        REQUIRE(bExtracted.b != b.b);

        pool.extract<TestStructA>(aDest, &aExtracted);
        pool.extract<TestStructB>(bDest, &bExtracted);

        REQUIRE(aExtracted.a == a.a);
        REQUIRE(bExtracted.a == b.a);
        REQUIRE(bExtracted.b == b.b);
    } END_LITL_TEST_CASE
}