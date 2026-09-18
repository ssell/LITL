#include "litl-import/texture/intermediate/litlbtex.hpp"

namespace litl::import
{
    bool LitlTextureBinary::serialize(TextureIntermediateData const& texture, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        // ... todo ...
        error = ErrorCode::FunctionNotImplemented;
        return false;
    }

    bool LitlTextureBinary::deserialize(TextureIntermediateData const& texture, ErrorCode& error) const noexcept
    {
        // ... todo ...
        error = ErrorCode::FunctionNotImplemented;
        return false;
    }
}