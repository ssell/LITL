#include "litl-import/material/intermediate/litlmatb.hpp"

namespace litl::import
{

    bool LitlMatBinary::serialize(MaterialIntermediateData const& material, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        return false;
    }

    bool LitlMatBinary::deserialize(MaterialIntermediateData& material, ErrorCode& error) const noexcept
    {
        return false;
    }
}