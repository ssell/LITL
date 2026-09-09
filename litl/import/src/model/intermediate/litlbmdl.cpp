#include "litl-import/model/intermediate/litlbmdl.hpp"

namespace litl::import
{
    bool LitlModelBinary::serialize(ModelIntermediateData const& model, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        return false;
    }

    bool LitlModelBinary::deserialize(ModelIntermediateData& model, ErrorCode& error) const noexcept
    {
        return false;
    }
}