#ifndef LITL_IMPORT_MODEL_LITLBMDL_H__
#define LITL_IMPORT_MODEL_LITLBMDL_H__

#include <vector>
#include "litl-import/model/intermediate/modelIntermediateData.hpp"

namespace litl::import
{
    struct LitlModelBinary final
    {
        enum class ErrorCode
        {

        };

        [[nodiscard]] static bool serialize(ModelIntermediateData const& model, std::vector<std::byte>& data, ErrorCode& error) noexcept;
        [[nodiscard]] bool deserialize(ModelIntermediateData& model, ErrorCode& error) const noexcept;
    };
}

#endif