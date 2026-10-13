#include "litl-core/directory.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-import/model/export/modelExporter.hpp"
#include "litl-import/model/intermediate/litlmdl.hpp"

namespace litl::import
{
    ModelExporter::ModelExporter()
    {

    }

    ModelExporter::~ModelExporter()
    {

    }

    Result ModelExporter::prepare(ImportedData& data, ImportSettings const& settings, uint32_t dataIndex) noexcept
    {
        auto* modelResult = data.items[dataIndex].getDataPtr<ModelImportResult>();

        if ((modelResult == nullptr) || (modelResult->model == nullptr))
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        auto* model = modelResult->model.get();

        if (modelResult->importConvention.sourceIsRightHanded)
        {
            // Negate z-values in all transform matrices to convert to left-handed transforms
            for (auto& node : model->getNodesWriteRef())
            {
                node.localTransform = mat4(node.localTransform).negateZValues().toArray();
            }
        }

        return Result::Success();
    }

    Result ModelExporter::write(std::vector<std::byte>& serialized, ImportedData const& data, uint32_t dataIndex) noexcept
    {
        auto errorCode = LitlModel::ErrorCode::None;
        auto* modelResult = data.items[dataIndex].getDataPtr<ModelImportResult>();

        if ((modelResult == nullptr) || (modelResult->model == nullptr))
        {
            return Result::Error(ErrorType::ImportedDataNull);
        }

        if (!LitlModel::serialize(*modelResult->model, serialized, errorCode))
        {
            return Result::Error(ErrorType::SerializationFailed, std::format("Serialized of ModelIntermediateData to LitlModel failed with error code {}", static_cast<uint32_t>(errorCode)));
        }

        if (serialized.empty())
        {
            return Result::Error(ErrorType::SerializedResultEmpty);
        }

        return Result::Success();
    }
}
