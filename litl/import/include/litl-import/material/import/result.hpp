#ifndef LITL_IMPORT_MATERIAL_RESULT_H__
#define LITL_IMPORT_MATERIAL_RESULT_H__

#include <memory>
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::import
{
    struct MaterialImportResult
    {
        std::shared_ptr<MaterialIntermediateData> intermediateMaterial;
    };
}

#endif