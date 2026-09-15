#include <glaze/json.hpp>
#include "litl-import/model/intermediate/litlmdl.hpp"

namespace litl::import
{
    namespace
    {

    }

    bool LitlModel::serialize(ModelIntermediateData const& model, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        std::string jsonBuffer{};
        const auto glzError = glz::write_json(model.getModel(), jsonBuffer);

        if (glzError != glz::error_code::none)
        {
            error = ErrorCode::ModelToJsonConversionFailed;
            return false;
        }

        data.resize(jsonBuffer.size(), std::byte{ 0 });
        std::memcpy(data.data(), reinterpret_cast<const std::byte*>(jsonBuffer.data()), jsonBuffer.size());

        return true;
    }

    bool LitlModel::deserialize(ModelIntermediateData& model, std::span<std::byte const> data, ErrorCode& error) noexcept
    {
        model.clear();

        if (data.empty())
        {
            error = ErrorCode::EmptyDeserializationData;
            return false;
        }

        std::string jsonBuffer{};
        jsonBuffer.resize(data.size(), static_cast<char>(0));
        std::memcpy(jsonBuffer.data(), reinterpret_cast<const char*>(data.data()), data.size_bytes());

        const auto glzError = glz::read_json(model.getModelWriteRef(), jsonBuffer);

        if (glzError != glz::error_code::none)
        {
            error = ErrorCode::JsonToModelConversionFailed;
            return false;
        }

        return true;
    }
}