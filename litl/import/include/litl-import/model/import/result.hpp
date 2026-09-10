#ifndef LITL_IMPORT_MODEL_RESULT_H__
#define LITL_IMPORT_MODEL_RESULT_H__

#include <memory>
#include "litl-import/model/intermediate/modelIntermediateData.hpp"

namespace litl::import
{
    struct ModelImportResult
    {
        std::unique_ptr<ModelIntermediateData> model;
    };
}

#endif