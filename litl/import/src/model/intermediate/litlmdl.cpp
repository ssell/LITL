#include <glaze/json.hpp>
#include "litl-import/model/intermediate/litlmdl.hpp"

namespace litl::import
{
    namespace
    {

    }

    bool LitlModel::parse(std::span<std::byte const> data, LitlModel& file, ErrorCode& error) noexcept
    {
        return false;
    }

    bool LitlModel::serialize(ModelIntermediateData const& model, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        std::string jsonBuffer{};
        const auto glzError = glz::write_json(model.getModel(), jsonBuffer);

        if (glzError != glz::error_code::none)
        {
            error = ErrorCode::JsonConversionFailed;
            return false;
        }

        data.resize(jsonBuffer.size(), std::byte{ 0 });
        std::memcpy(data.data(), reinterpret_cast<const std::byte*>(jsonBuffer.data()), jsonBuffer.size());

        return true;
    }

    bool LitlModel::deserialize(ModelIntermediateData& model, ErrorCode& error) const noexcept
    {
        return false;
    }
}