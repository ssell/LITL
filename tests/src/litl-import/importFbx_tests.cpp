#include "tests.hpp"
#include "litl-import/importService.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Import FBX", "[import::fbx]")
    {
        constexpr std::string_view location = "assets/mesh/cube.fbx";
        const File source(location);

        REQUIRE(source.exists() == true);

        const auto bytes = source.readAllBytes();

        REQUIRE(bytes.has_value());

        import::ImportService importer{};
        import::ImportedData data{};
        const import::Result result = importer.importForMemory(import::ImportSourceType::ModelFbx, location, bytes.value(), data, true);

        // todo ...
        //REQUIRE(result.success == true);
        //REQUIRE(result.error == import::ErrorType::None);
        //REQUIRE(data.type == import::ImportedDataType::Mesh);
        //REQUIRE(data.mesh != nullptr);

    } LITL_END_TEST_CASE
}