#include <cstring>

#include "tests.hpp"
#include "litl-import/importService.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Import FBX", "[import::fbx]")
    {
        const File source("assets/mesh/cube.fbx");

        REQUIRE(source.exists() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.import(source, data, true);

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
        REQUIRE(data.type == import::ImportedDataType::Mesh);
        REQUIRE(data.mesh != nullptr);

        // todo ...
    } LITL_END_TEST_CASE
}