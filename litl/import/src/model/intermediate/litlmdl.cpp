#include "litl-import/model/intermediate/litlmdl.hpp"

namespace litl::import
{
    bool LitlModel::serialize(ModelIntermediateData const& model, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        return false;
    }

    bool LitlModel::deserialize(ModelIntermediateData& model, ErrorCode& error) const noexcept
    {
        return false;
    }
}