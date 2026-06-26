#include "tests.hpp"
#include "litl-core/containers/alignedByteBuffer.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Alignment of 1", "[core::containers::alignedByteBuffer]")
    {
        constexpr size_t itemCount = 10;
        constexpr size_t sizeBytes = sizeof(bool) * itemCount;

        AlignedByteBuffer<alignof(bool)> bufferBool{ sizeBytes };
        REQUIRE(bufferBool.size() == sizeBytes);

        bool* writePtr = bufferBool.as<bool>().data();

        // Set to alternating true/false
        for (size_t i = 0; i < itemCount; ++i)
        {
            *writePtr = ((i % 2) == 0);
            writePtr++;
        }

        bool const* readPtr = bufferBool.as<bool>().data();

        for (size_t i = 0; i < itemCount; ++i)
        {
            REQUIRE(*readPtr == ((i % 2) == 0));
            readPtr++;
        }

        // auto thisWouldFailCompileTimeCheckAsUint32AligmentIsTooGreat = bufferBool.as<uint32_t>();

    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Alignment of 4", "[core::containers::alignedByteBuffer]")
    {
        constexpr size_t itemCount = 5;
        constexpr size_t sizeBytes = sizeof(uint32_t) * itemCount;

        AlignedByteBuffer<alignof(uint32_t)> bufferUint32{ sizeBytes };
        REQUIRE(bufferUint32.size() == sizeBytes);

        uint32_t* writePtr = bufferUint32.as<uint32_t>().data();

        for (uint32_t i = 0u; i < static_cast<uint32_t>(itemCount); ++i)
        {
            *writePtr = i;
            writePtr++;
        }

        uint32_t const* readPtr = bufferUint32.as<uint32_t>().data();

        for (uint32_t i = 0u; i < static_cast<uint32_t>(itemCount); ++i)
        {
            REQUIRE(*readPtr == i);
            readPtr++;
        }

        // auto thisWouldFailCompileTimeCheckAsUint64AligmentIsTooGreat = bufferUint32.as<uint64_t>();
    } LITL_END_TEST_CASE
}