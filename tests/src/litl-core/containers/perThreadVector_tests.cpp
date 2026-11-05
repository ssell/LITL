#include "tests.hpp"
#include "litl-core/containers/perThreadVector.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("tt", "[core::containers::perThreadVector]")
    {
        PerThreadVector<uint32_t> vectors;
    } LITL_END_TEST_CASE
}