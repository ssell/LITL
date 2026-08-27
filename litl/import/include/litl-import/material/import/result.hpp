#ifndef LITL_IMPORT_MATERIAL_RESULT_H__
#define LITL_IMPORT_MATERIAL_RESULT_H__

#include <memory>
#include "litl-import/material/intermediate/litlmatb.hpp"

namespace litl::import
{
    struct MaterialImportResult
    {
        std::unique_ptr<LitlMatBinary> material;
    };
}

#endif