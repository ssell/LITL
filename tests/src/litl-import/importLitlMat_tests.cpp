#include "tests.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("Import litlmat", "[import::litlmat]")
    {
        File source("assets/materials/flat.litlmat");

        REQUIRE(source.exists() == true);

        import::ImportService importer{};
        import::ImportedData data{};
        import::Result const result = importer.import(source, data, true);

        REQUIRE(result.success == true);

        // ... todo ...
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("Convert litlmat to litlmatb", "[import::litlmat]")
    {
        File source("assets/materials/flat.litlmat");
        File dest("assets/materials/flat.litlmatb");

        REQUIRE(source.exists() == true);

        if (dest.exists() == true)
        {
            dest.erase();
            REQUIRE(dest.exists() == false);
        }

        // Test full conversion (litlmat -> MaterialIntermediateData -> litlmatb)
        import::ImportService importer{};
        import::Result result = importer.convert(source.absolutePath());

        REQUIRE(result.success == true);
        REQUIRE(result.error == import::ErrorType::None);
    } LITL_END_TEST_CASE
}