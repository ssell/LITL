#ifndef LITL_IMPORT_SHADER_RESULT_H__
#define LITL_IMPORT_SHADER_RESULT_H__

#include <memory>
#include "litl-import/shader/intermediate/shaderIntermediateData.hpp"

namespace litl::import
{
    struct ShaderImportResult
    {
        std::shared_ptr<ShaderIntermediateData> intermediateShader;
    };
}

#endif