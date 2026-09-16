#include <format>

#include "litl-core/directory.hpp"
#include "litl-import/material/export/materialExporter.hpp"
#include "litl-import/material/intermediate/litlbmat.hpp"

namespace litl::import
{
    MaterialExporter::MaterialExporter()
    {

    }

    MaterialExporter::~MaterialExporter()
    {

    }

    Result MaterialExporter::prepare(ImportedData& data, uint32_t dataIndex) noexcept
    {
        if (dataIndex >= data.items.size())
        {
            return Result::Error(ErrorType::InvalidImportedItemIndex);
        }

        auto& dataItem = data.items[dataIndex];

        if (dataItem.getType() != ImportedDataType::Material)
        {
            return Result::Error(ErrorType::ImportedDataTypeMismatch);
        }

        auto* material = dataItem.getDataPtr<MaterialImportResult>();

        if (material == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (material->intermediateMaterial == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        return Result::Success();
    }

    Result MaterialExporter::write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept
    {
        auto errorCode = BinaryBlockFile::ErrorCode::None;
        auto* material = data.items[dataIndex].getDataPtr<MaterialImportResult>();

        if (material == nullptr)
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        MaterialIntermediateData* intermediateMaterial = material->intermediateMaterial.get();

        if (!LitlMatBinary::serialize(*intermediateMaterial, serialized, errorCode))
        {
            return Result::Error(ErrorType::SerializationFailed, std::format("Serialization of Material to litlbmat failed with error code {}", static_cast<uint32_t>(errorCode)));
        }

        if (serialized.empty())
        {
            return Result::Error(ErrorType::SerializedResultEmpty);
        }

        return Result::Success();
    }
}